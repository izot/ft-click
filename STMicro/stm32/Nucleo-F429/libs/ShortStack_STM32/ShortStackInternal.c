/*
 * Filename: ShortStackInternal.c
 *
 * Description: This file contains the implementation of some internal 
 * functions used by the ShortStack API. 
 *
 * Copyright (c) Echelon Corporation 2002-2009.  All rights reserved.
 *
 * This file is ShortStack LonTalk Compact API Software as defined in the 
 * Software License Agreement that governs its use.
 *
 * ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
 * ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
 * ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
 * QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE,
 * NONINFRINGEMENT, AND THEIR EQUIVALENTS.
 */

#include <string.h>
#include "ShortStackDev.h"
#include "ShortStackApi.h"

/* 
 * Following are a few macros that are used internally to access 
 * network variable and application message details.
 */
#define PSICB   ((LonSicb*)pSmipMsg->Payload)
#define EXPMSG  (PSICB->ExplicitMessage)
#define NVMSG   (PSICB->NvMessage)

/* 
 * Forward declarations for functions used internally by the ShortStack Api.
 */
const LonApiError VerifyNv(const LonByte nvIndex, LonByte nvLength);
void  PrepareNvMessage(LonSmipMsg* pSmipMsg, const LonByte nvIndex, const LonByte* const pData, const LonByte len);
const LonApiError SendNv(const LonByte nvIndex);
const LonApiError SendNvPollResponse(const LonSmipMsg* pSmipMsg);
const LonApiError SendLocal(const LonSmipCmd command, const void* const pData, const LonByte length);
const LonApiError WriteNvLocal(const LonByte index, const void* const pData, const LonByte length);

/***********************************************************************************
 * VerifyNv
 *
 * This is a utility to verify the validity of a local network variable index and
 * this network variable's length.
 *
 * Arguments:  index -- index of the network variable; will be checked for validity
 *             length -- the alleged length of this network variable, will be checked
 *                      if positive. For supported negative values, see VERIFY_* below.
 *
 *             You can call this function with the assumed positive length in its second
 *             argument, and both index and length will be verified. In this case, the
 *             assumed length must equal the reported current length.
 *
 *             You can call this function with 0 for the second argument.
 *             This causes the length validation to be skipped.
 *
 * Returns:    lonApiNoError for success, or an appropriate error code otherwise
 ********************************************************************************** */

const LonApiError VerifyNv(const LonByte nvIndex, LonByte nvLength) 
{
    LonApiError result = LonApiNoError;

    if (nvLength > LON_MAX_MSG_NV_DATA) 
    {
        /* Return failure if NV data is too big */
        result = LonApiNvLengthTooLong;
    } 
    else if (LonNvCount <= 0 || nvIndex > (LonNvCount - 1)) 
    {
        /* we can only handle an NV with a valid index... */
        result = LonApiNvIndexInvalid;
    } 
    else 
    {
        /* now we have a known good index. Let's check the reported length: */
        if (nvLength > 0 && nvLength != LonGetCurrentNvSize(nvIndex)) 
        {
            /* 
             * Reject a bad length. This could be caused by some API error, or an incorrect
             * NV table, or a changeable type network variable whose current length is not
             * correctly reported by the LonGetNvSize() callback function.
             */
            result = LonApiNvLengthMismatch;
        }
    }
    return result;
}

/***********************************************************************************
 * PrepareNvMessage
 *
 * Description: Prepares a generic network variable message, taking 
 *              care of cases when the nv index is greater than 63.
 *
 * Arguments:   pSmipMsg -- pointer to the message to be prepared
 *              nvIndex -- index of the network variable to send
 *              pData   -- pointer to the network variable data
 *              len     -- length of the network variable data
 *
 * Returns:     Non-zero error code if an error occurs.
 *              Zero if the message was prepared successfully.
 **********************************************************************************/
void PrepareNvMessage(LonSmipMsg* pSmipMsg, const LonByte nvIndex, const LonByte* const pData, const LonByte len)
{
    memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
    /* if the nv index is less than 63, it is also stored in the command byte */
    pSmipMsg->Header.Command = (LonSmipCmd) (nvIndex < LON_NV_ESCAPE_SEQUENCE ? (LonNiNv | nvIndex) : (LonNiNv | LON_NV_ESCAPE_SEQUENCE));
    NVMSG.Index = nvIndex;
    LON_SET_ATTRIBUTE(NVMSG, LON_NVMSG_MSGTYPE, LonMessageNv);
    NVMSG.Length = len;
    if (len  &&  pData != NULL)
        memcpy(NVMSG.NvData, pData, len);
    pSmipMsg->Header.Length = (LonByte) (sizeof(LonNvMessage) - sizeof(NVMSG.NvData) + len);
}

/***********************************************************************************
 * SendNv
 *
 * Description: Send a network variable update message onto the network.
 *
 * Arguments:   index -- index of the network variable to send
 *              pValue  -- pointer to the network variable data
 *
 * Returns:     Non-zero error code if an error occurs.
 *              Zero if the outgoing NV-update message has been buffered by the driver.
 *
 * Caveats:     On success, this will result in network traffic.
 **********************************************************************************/
const LonApiError SendNv(const LonByte nvIndex)
{
    LonApiError result = VerifyNv(nvIndex, LonGetCurrentNvSize(nvIndex));

    if (result == LonApiNoError) 
    {
        LonSmipMsg* pSmipMsg = NULL;
        if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError) 
        {
            /* Return failure if transmit buffer is full */
            result = LonApiTxBufIsFull;
        } 
        else 
        {
            const LonNvDescription* const pNvTable = LonGetNvTable();
            PrepareNvMessage(pSmipMsg, nvIndex, (LonByte *) pNvTable[nvIndex].pData, (LonByte) LonGetCurrentNvSize(nvIndex));
            LdvPutMsg(pSmipMsg);
        }
    }
    return result;
}

/***********************************************************************************
 * SendNvPollResponse
 *
 * Description: Send a response to an NV-poll request.
 *
 * Arguments:   pSmipMsg -- incoming NV-poll request message
 *
 * Returns:     Non-zero error code if an error occurs.
 *              Zero if the NV-poll response message has been buffered by the driver.
 **********************************************************************************/
const LonApiError SendNvPollResponse(const LonSmipMsg* pSmipMsg)
{
    const unsigned nvIndex = NVMSG.Index;
    LonApiError result = VerifyNv((LonByte) nvIndex, LonGetCurrentNvSize(nvIndex));

    if (result == LonApiNoError) 
    {
        LonSmipMsg* pSmipResponse = NULL;
        if ((result = LdvAllocateMsg(&pSmipResponse)) == LonApiNoError) 
        {
            const unsigned aliasIndex = NVMSG.AliasIndex;
            const unsigned nvLength = LonGetCurrentNvSize(nvIndex);
            const LonNvDescription* const nvTable = LonGetNvTable();
            LonNvMessage* const pNvResponse = (LonNvMessage* const)pSmipResponse->Payload;

            // todo ekh 1 - I am sure that they did not mean to set just the size of the pointer to zero ! or did they?
            // why not pSmipResponse = NULL ; ??
            // or did they mean sizeof(LonSmipMsg)
            memset(pSmipResponse, 0, sizeof(pSmipResponse));

            /* Copy the correlator fields namely Tag, MsgType and Priority. */
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_TAG, LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_TAG));
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_MSGTYPE, LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_MSGTYPE));
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_PRIORITY, LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_PRIORITY));
            /* Set the response attribute */
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_RESPONSE, 1);
            pNvResponse->Length = (LonByte) nvLength;
            /* If alias index is valid, i.e., it is not 128, then treat as alias. */
            pNvResponse->Index = (LonByte) ((aliasIndex & 0x80) ? nvIndex : aliasIndex); /* To Do: Define a macro for 0x80 */
            pNvResponse->AliasIndex = aliasIndex;
            memcpy(pNvResponse->NvData, (const void*) nvTable[nvIndex].pData, nvLength);
            pSmipResponse->Header.Length = (LonByte) (sizeof(LonNvMessage) - sizeof(NVMSG.NvData) + nvLength);
            pSmipResponse->Header.Command = (LonSmipCmd) (nvIndex < LON_NV_ESCAPE_SEQUENCE ? (LonNiNv | nvIndex) : (LonNiNv | LON_NV_ESCAPE_SEQUENCE));
            LdvPutMsg(pSmipResponse);
        }
    }
    return result;
}


/***********************************************************************************
 * SendLocal
 *
 * Description: Send a local network interface command to the ShortStack Micro Server
 *
 * Arguments:   command -- network interface command to send
 *
 * Returns:     None-Zero error code if an error occurs.
 *              Zero if the local NI command message has been buffered by the driver.
 **********************************************************************************/
const LonApiError SendLocal(const LonSmipCmd command, const void* const pData, const LonByte length)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError) 
    {
        /* return failure if buffer unavailable */
        result = LonApiTxBufIsFull;
    } 
    else 
    {
        /* OK, construct and post message: */
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
        pSmipMsg->Header.Length = length;
        pSmipMsg->Header.Command = command;
        memcpy(PSICB, pData, length);
    
        LdvPutMsg(pSmipMsg);
    }
    return result;
}

/***********************************************************************************
 * WriteNvLocal
 *
 * Description: Write network variable value locally. This is called when an NV
 *              update or non-zero NV poll response arrives.
 *
 * Arguments:   index -- index of the network variable to be written.
 *              pData   -- pointer to the network variable value
 *              length -- length of network variable value
 *
 * Returns:     Zero if the NV was updated successfully.
 *              None-zero error code if an error occurs
 **********************************************************************************/
const LonApiError WriteNvLocal(const LonByte index, const void* const pData, const LonByte length)
{
    LonApiError result = VerifyNv(index, length);

    if (result == LonApiNoError) 
    {
        /* OK, update this input NV in the application space: */
        const LonNvDescription* const pNvTable = LonGetNvTable();
        memcpy((void*) pNvTable[index].pData, pData, length);
    }
    return result;
}
