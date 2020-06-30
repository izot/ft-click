/*
 * ShortStackApi.c
 *
 * Description: This file contains function implementations for the ShortStack 
 * LonTalk Compact API. 
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

#if LON_ISI_ENABLED
#include "ShortStackIsiApi.h"
#endif /* LON_ISI_ENABLED */

/* 
 * Following are a few macros that are used internally to access 
 * network variable and application message details.
 */
#define PSICB   ((LonSicb*)pSmipMsg->Payload)
#define EXPMSG  (PSICB->ExplicitMessage)
#define NVMSG   (PSICB->NvMessage)

/*
 * Following is the reset message buffer. Any uplink reset message will be copied
 * into this buffer, which serves as a source for validation of various indices 
 * against the Micro Server's capabilities, and as a source for version number and
 * Micro Server Unique Id (Neuron Id). At reset, the buffer is cleared, 
 * indicating that the remaining information in this buffer is invalid.
 */
static volatile LonResetNotification lastResetNotification;

/* 
 * A global array to hold the message response data.
 * Make no assumptions about the previous contents while using it.  
 * If required, NULL out all data before use.
 */
LonByte ResponseData[LON_MAX_MSG_DATA];

/* 
 * Typedef used to keep track of local NM/ND messages.
 * The LonTalk protocol has a limitation that the response codes for these 
 * messages are not unique. Hence, the API limits only one such request pending 
 * at any given time and keeps track of the type of the message. 
 */
typedef enum
{
    NO_NM_ND_PENDING,
    NM_PENDING,
    ND_PENDING
}NmNdStatus;
NmNdStatus CurrentNmNdStatus = NO_NM_ND_PENDING;

/* 
 * Forward declarations for functions used internally by the ShortStack Api.
 * These functions are implemented in ShortStackInternal.c.  
 */
extern const LonApiError VerifyNv(const LonByte nvIndex, LonByte nvLength);
extern void  PrepareNvMessage(LonSmipMsg* pSmipMsg, const LonByte nvIndex, const LonByte* const pData, const LonByte len);
extern const LonApiError SendNv(const LonByte nvIndex);
extern const LonApiError SendNvPollResponse(const LonSmipMsg* pSmipMsg);
extern const LonApiError SendLocal(const LonSmipCmd command, const void* const pData, const LonByte length);
extern const LonApiError WriteNvLocal(const LonByte index, const void* const pData, const LonByte length);

#if LON_ISI_ENABLED
extern LonApiError SendDownlinkRpc(IsiDownlinkRpcCode code, LonByte param1, LonByte param2, void* pData, unsigned len);
extern void HandleDownlinkRpcAck(IsiRpcMessage* pMsg, LonBool bSuccess);
extern void HandleUplinkRpc(IsiRpcMessage* pMsg);
#endif /* LON_ISI_ENABLED */

/*
 * Function: LonInit
 * Initializes the ShortStack LonTalk Compact API and Micro Server
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * The function has no parameters.  It returns a <LonApiError> code to indicate
 * success or failure. The function must be called during application 
 * initialization, and prior to invoking any other function of the ShortStack 
 * LonTalk Compact API. 
 * Note that the Micro Server disables all network communication until this 
 * function completes successfully.
 */
const LonApiError LonInit(const char *usartName )
{
    LonApiError result;
    LonFrameworkInit();
    /* Clear the information obtained from the last reset notification message */
    memset((void*) &lastResetNotification, 0, sizeof(LonResetNotification));
    
    /* Initialize the serial driver */
    LdvInit( usartName );
    
    /* Read the NV values (if any) from persistent storage  */
    result = LonNvdDeserializeNvs();
    
    /* Send the Initialization data to the ShortStack Micro Server */
    if (result == LonApiNoError)
    {
        LonSmipMsg smipMsg;
        LonByte nTotalNvCount;
        LonByte nTotalNvsSent;
        
        const LonByte *pInitData = LonGetAppInitData();
        
        /* The LonGetAppInitData function returns a structure containing the application 
           initialization data followed by the network variable initialization data. 
           The structure needs to be parsed to extract this data and then needs to be formatted. */
        
        /* Prepare and send the LonInitialization message to the ShortStack Micro Server */
        memset(&smipMsg, 0, sizeof(smipMsg));
        smipMsg.Header.Command = LonNiAppInit;
        smipMsg.Header.Length = LON_APP_INIT_MSG_SIZE;
        memcpy(smipMsg.Payload, pInitData, LON_APP_INIT_MSG_SIZE);
        result = LdvPutMsgBlocking(&smipMsg);
        
        /* Prepare and send the LonNvInitialization messages to the ShortStack Micro Server */
        nTotalNvCount = pInitData[LON_APP_INIT_MSG_SIZE - 1]; /* The last byte of the app init message contains the Nv Count */
        nTotalNvsSent = 0;
        while (result == LonApiNoError)
        {
            LonByte nStartIndex;
            LonByte nStopIndex;
            
            /* Calculate the number of nvs that can be sent in this message */
            nStartIndex = nTotalNvsSent;
            nStopIndex = ((nTotalNvCount - nTotalNvsSent) > LON_MAX_NVS_IN_NV_INIT) ? (nStartIndex + LON_MAX_NVS_IN_NV_INIT) : nTotalNvCount;
    
            memset(&smipMsg, 0, sizeof(smipMsg));
            smipMsg.Header.Command = LonNiNvInit; 
            smipMsg.Header.Length = 3 + nStopIndex - nStartIndex; 
            smipMsg.Payload[0] = nStartIndex; 
            smipMsg.Payload[1] = nStopIndex; 
            smipMsg.Payload[2] = nTotalNvCount; 
            memcpy(smipMsg.Payload + 3, pInitData + LON_APP_INIT_MSG_SIZE + nTotalNvsSent, nStopIndex - nStartIndex);
            result = LdvPutMsgBlocking(&smipMsg);
            nTotalNvsSent += (nStopIndex - nStartIndex);
            if (nTotalNvsSent == nTotalNvCount)
                break;
        }
    }
    
    /* Reset the Micro Server so that any configuration change to the ShortStack    */
    /* Micro Server can take effect.                                                */
    if (result == LonApiNoError)
    {
        LonSmipMsg smipMsg;

        memset(&smipMsg, 0, sizeof(smipMsg));

        smipMsg.Header.Command = LonNiReset;
        smipMsg.Header.Length = 0;

        result = LdvPutMsgBlocking(&smipMsg);
    }

    return result;
}

/*
 * Function: LonEventHandler
 * Periodic service to the ShortStack LonTalk Compact API. 
 *
 * Remarks:
 * This function must be called periodically by the application.  This
 * function processes any messages that have been received from the Micro Server.
 * The application can call this function as part of the idle loop, or from a 
 * dedicated timer-based thread or interrupt service routine. All API callback 
 * functions occur within this function's context. Note that the application is 
 * responsible for correct context management and thread synchronization, as 
 * (and if) required by the hosting platform.
 *
 * This function must be called at least once every 10 ms.  Use the following 
 * formula to determine the minimum call rate:
 *  rate = MaxPacketRate / (InputBufferCount - 1) 
 * where MaxPacketRate is the maximum number of packets per second arriving for 
 * the device and InputBufferCount is the number of input buffers defined for 
 * the application.
 */
void LonEventHandler(void)
{
    LonSmipMsg* pSmipMsg = NULL;

    /* Force the serial driver to flush its transmit buffers */
    LdvFlushMsgs();

    if (LdvGetMsg(&pSmipMsg) == LonApiNoError)
    {
        /* A message has been retrieved from driver's receive buffer    */
        LonCorrelator correlator = {0};
        
        /* Make correlation structure    */
        LON_SET_ATTRIBUTE(correlator, LON_CORRELATOR_PRIORITY, LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY));
        LON_SET_ATTRIBUTE(correlator, LON_CORRELATOR_TAG, LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG));
        LON_SET_ATTRIBUTE(correlator, LON_CORRELATOR_SERVICE, LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE));

        switch (pSmipMsg->Header.Command)
        {
            case ((LonByte) LonNiComm | (LonByte) LonNiIncoming):
            {
                /* Is an incoming message    */
                LonBool bFailure = FALSE;

                if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE) == LonMessageNv)
                {
                    /* Process NV messages */
                    if (LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_NVPOLL))
                    {
                        /* Process NV poll message */
                        bFailure = (SendNvPollResponse(pSmipMsg) != LonApiNoError);
                    }
                    else if (WriteNvLocal(NVMSG.Index, NVMSG.NvData, NVMSG.Length) == LonApiNoError) 
                    {
                        /* Process NV update message */
                        #if LON_EXPLICIT_ADDRESSING
                            LonNvUpdateOccurred(NVMSG.Index, &(EXPMSG.Address.Receive));
                        #else 
                            LonNvUpdateOccurred(NVMSG.Index, NULL);
                        #endif /* LON_EXPLICIT_ADDRESSING */
                    }
                }
                else
                {
                    /* Process explicit messages */
                    switch (EXPMSG.Code)
                    {
                        case LonNmSetNodeMode:
                            /* Process Set Node Mode network management message    */
                            switch(EXPMSG.Data.NodeMode.Mode)
                            {
                                case LonApplicationOffLine:
                                    LonOffline();
                                    SendLocal(LonNiOffLine, NULL, 0);
                                    break;
                                case LonApplicationOnLine:
                                    LonOnline();
                                    SendLocal(LonNiOnLine, NULL, 0);
                                    break;
                                default:
                                    bFailure = TRUE;
                                    break;
                            }
                            break;

                        case LonNmNvFetch:
                            /* Process Nv Fetch network management message        */
                            if (EXPMSG.Data.NvFetch.Index == 0xFF)
                                /* This is the escape index which means that the true index is 
                                   255 or greater and is in the following two bytes. 
                                   ShortStack doesn't support NV's with index greater than 254. */
                                bFailure = TRUE;
                            else 
                            {
                                const unsigned nvIndex = EXPMSG.Data.NvFetch.Index;
                                const unsigned nvLength = LonGetCurrentNvSize(nvIndex);
                                if (VerifyNv(nvIndex, nvLength) != LonApiNoError) 
                                    bFailure = TRUE;
                                else
                                {
                                    /* Send NV response to the network.           */
                                    const LonNvDescription* const nvTable = LonGetNvTable();
    
                                    ResponseData[0] = (LonByte) nvIndex;
                                    memcpy(&ResponseData[1], (const void*) nvTable[nvIndex].pData, nvLength);
                                    bFailure = (LonSendResponse(correlator, LON_NM_SUCCESS(LonNmNvFetch), ResponseData, nvLength + 1) != LonApiNoError);
                                }
                            }
                            break;
                            
                        #if     LON_DMF_ENABLED    
                        case LonNmReadMemory:
                            /* Process Read Memory network management message        */
                            if (EXPMSG.Data.ReadMemory.Mode != LonAbsoluteMemory) 
                                bFailure = TRUE;
                            else if ((LonMemoryRead(LON_GET_UNSIGNED_WORD(EXPMSG.Data.ReadMemory.Address), 
                                              EXPMSG.Data.ReadMemory.Count, &ResponseData[0]) == LonApiNoError)
                                    && (LonSendResponse(correlator, LON_NM_SUCCESS(LonNmReadMemory), 
                                        &ResponseData[0], EXPMSG.Data.ReadMemory.Count) == LonApiNoError))
                                bFailure = FALSE;
                            else
                                bFailure = TRUE;
                            break;
                            
                        case LonNmWriteMemory:
                            /* Process Write Memory network management message        */
                            if (EXPMSG.Data.WriteMemory.Mode != LonAbsoluteMemory) 
                                bFailure = TRUE;
                            else if ((LonMemoryWrite(LON_GET_UNSIGNED_WORD(EXPMSG.Data.WriteMemory.Address),
                                                EXPMSG.Data.WriteMemory.Count, 
                                                ((LonByte*) &EXPMSG.Data.WriteMemory.Form) + 1) == LonApiNoError)
                                    && (LonSendResponse(correlator, LON_NM_SUCCESS(LonNmWriteMemory), NULL, 0) == LonApiNoError))
                                bFailure = FALSE;
                            else
                                bFailure = TRUE;
                            break;
                        #endif      /* LON_DMF_ENABLED */
                            
                        case LonNmQuerySiData:
                            {
                                unsigned offset = LON_GET_UNSIGNED_WORD(EXPMSG.Data.QuerySiDataRequest.Offset);
                                unsigned siDataLength = 0;
                                const LonByte* pSiData = LonGetSiData(&siDataLength);
                                if (EXPMSG.Data.QuerySiDataRequest.Count > LON_MAX_MSG_DATA
                                    || offset + EXPMSG.Data.QuerySiDataRequest.Count > siDataLength) 
                                    bFailure = TRUE;
                                else 
                                {
                                      memcpy(&ResponseData[0], pSiData + offset, EXPMSG.Data.QuerySiDataRequest.Count);
                                      bFailure = (LonSendResponse(correlator, LON_NM_SUCCESS(LonNmQuerySiData), &ResponseData[0], 
                                                                  EXPMSG.Data.QuerySiDataRequest.Count) != LonApiNoError);
                                }
                            }
                            break;

                        case LonNmWink:
                            /* Process wink network management message        */
                            LonWink();
                            break;

                        default:
                            /* Process explicit application messages here.   */
                            #if    LON_APPLICATION_MESSAGES 
                                #if    LON_EXPLICIT_ADDRESSING
                                    LonMsgArrived(&(EXPMSG.Address.Receive), correlator, 
                                                  (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY),
                                                  (LonServiceType) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE),
                                                  (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED),
                                                  (LonApplicationMessageCode) EXPMSG.Code, EXPMSG.Data.Data, (LonByte)(EXPMSG.Length-1));
                                #else    /* ifndef(LON_EXPLICIT_ADDRESSING)    */
                                    LonMsgArrived(NULL, correlator, 
                                                  (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY),
                                                  (LonServiceType) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE),
                                                  (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED),
                                                  (LonApplicationMessageCode) EXPMSG.Code, EXPMSG.Data.Data, (LonByte)(EXPMSG.Length-1));
                                #endif    /* LON_EXPLICIT_ADDRESSING            */
                            #else     /* ifndef(LON_APPLICATION_MESSAGES)    */
                                bFailure = TRUE;
                            #endif    /* LON_APPLICATION_MESSAGES            */
                            break;
                    }
                }
                if (bFailure)
                {
                    /* Indicates that the receiving network management message   */
                    /* or explicit message is not supported by the ShortStack,   */
                    /* or that it failed to execute that message.                */
                    LonSendResponse(correlator, (LonByte)LON_NM_FAILURE(EXPMSG.Code), NULL, 0);
                }
                break;
            }

            case ((LonByte) LonNiComm | (LonByte) LonNiResponse):
            {
                if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_COMPLETIONCODE))
                {
                    /* Process completion event generated by the ShortStack Micro Server */
                    if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE) == LonMessageNv)
                    {
                        LonNvUpdateCompleted(NVMSG.Index, (LonBool)(LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_COMPLETIONCODE) == LonCompletionSuccess));
                    }
                    else
                    {
                        #if    LON_APPLICATION_MESSAGES
                            LonMsgCompleted(LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG), 
                                            (LonBool)(LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_COMPLETIONCODE) == LonCompletionSuccess));
                        #endif    /* LON_APPLICATION_MESSAGES */
                    }
                }
                else
                {
                    /* Process response from the network. */
                    if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE) == LonMessageNv)
                    {
                        /* NV poll response.  Handle same as NV update. 
                         * (An offline node will return an NV update with length 0 to 
                         * indicate this fact. If all NV updates are returned this way 
                         * then a failure completion event is received).
                         */
                        if (VerifyNv(NVMSG.Index, NVMSG.Length) == LonApiNoError) 
                        {
                            if (WriteNvLocal(NVMSG.Index, NVMSG.NvData, NVMSG.Length) == LonApiNoError)
                                #if LON_EXPLICIT_ADDRESSING
                                    LonNvUpdateOccurred(NVMSG.Index, &(EXPMSG.Address.Receive));
                                #else
                                    LonNvUpdateOccurred(NVMSG.Index, NULL);
                                #endif
                        }
                    } 
                    else 
                    {
                        /* Message response. This could be a response to a local NM/ND
                         * message or an explicit message. If the message tag of the
                         * response is NM_ND_TAG, it is the response to the local
                         * NM/ND message.
                         */
                        if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG) == NM_ND_TAG)
                        {
                            #if LON_NM_QUERY_FUNCTIONS
                            if (CurrentNmNdStatus == NM_PENDING)
                            {
                                /* Process the response to a local NM/ND message    */
                                switch ((EXPMSG.Code & LON_NM_OPCODE_MASK) | LON_NM_OPCODE_BASE)
                                {
                                    case LonNmQueryDomain:
                                        /* Query Domain response    */
                                        LonDomainConfigReceived((LonDomain*) &(EXPMSG.Data), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryDomain)));
                                        break;
                                    case LonNmQueryNvConfig:
                                        /* Query Nv Config response    */
                                        if (EXPMSG.Length == sizeof(EXPMSG.Code) + sizeof(LonNvConfig)) 
                                            LonNvConfigReceived((LonNvConfig*) &(EXPMSG.Data), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryNvConfig)));
                                        else 
                                            LonAliasConfigReceived((LonAliasConfig*) &(EXPMSG.Data), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryNvConfig)));
                                        break;
                                    case LonNmQueryAddr:
                                        /* Query Address response    */
                                        LonAddressConfigReceived((LonAddress*) &(EXPMSG.Data), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryAddr)));
                                        break;
                                    case LonNmReadMemory:
                                        /* Read of configuration data response    */
                                        LonConfigDataReceived((const LonConfigData* const)&(EXPMSG.Data.Data), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmReadMemory)));
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else if (CurrentNmNdStatus == ND_PENDING)
                            {
                                /* Process the response to a local NM/ND message    */
                                switch ((EXPMSG.Code & LON_ND_OPCODE_MASK) | LON_ND_OPCODE_BASE)
                                {
                                    case LonNdQueryStatus:
                                        /* Query Status response */
                                        LonStatusReceived(&(EXPMSG.Data.QueryStatusResponse.Status), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNdQueryStatus)));
                                        break;
                                    case LonNdQueryXcvr:
                                        /* Query Transceiver Status response */
                                        LonTransceiverStatusReceived(&(EXPMSG.Data.QueryXcvrStatusResponse.Status), (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNdQueryXcvr)));
                                        break;
                                    default:
                                        break;
                                }
                            }
                            #endif  /* LON_NM_QUERY_FUNCTIONS */
                            CurrentNmNdStatus = NO_NM_ND_PENDING;
                        }
                        else
                        {
                            /* Explicit message response. */
                            #if    LON_APPLICATION_MESSAGES
                                #if    LON_EXPLICIT_ADDRESSING
                                    LonResponseArrived(&(EXPMSG.Address.Response), 
                                                       LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG), 
                                                       (LonApplicationMessageCode) EXPMSG.Code, EXPMSG.Data.Data, (LonByte)(EXPMSG.Length-1));
                                #else
                                    LonResponseArrived(NULL, 
                                                       LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG), 
                                                       (LonApplicationMessageCode) EXPMSG.Code, EXPMSG.Data.Data, (LonByte)(EXPMSG.Length-1));
                                #endif
                            #endif    /* LON_APPLICATION_MESSAGES    */
                        }
                    }
                }
                break;
            }

            case LonNiReset:
                /* The ShortStack Micro Server resets.    */
                /* Reset the serial driver to get back in sync. */
                LdvReset();
                CurrentNmNdStatus = NO_NM_ND_PENDING;
                memcpy((void*)&lastResetNotification, (LonResetNotification *) pSmipMsg, sizeof(LonResetNotification)); 
                LonResetOccurred((LonResetNotification *) pSmipMsg);
                break;

            case LonNiService:
                /* Service pin was  pressed.*/
                LonServicePinPressed();
                break;

            case LonNiServiceHeld:
                /* Service pin has been held longer than a configurable period of time. */
                /* See ShortStack User's Guide on how to set the period.                */
                LonServicePinHeld();
                break;
                
            #if LON_UTILITY_FUNCTIONS
            case LonNiUsop:
                /* A response to one of the utility functions has arrived. */
                switch (pSmipMsg->Payload[0])
                {
                    case LonUsopPing:
                        LonPingReceived();
                        break;
                        
                    case LonUsopNvIsBound:
                        LonNvIsBoundReceived(pSmipMsg->Payload[1], (LonBool) pSmipMsg->Payload[2]);
                        break;
                        
                    case LonUsopMtIsBound:
                        LonMtIsBoundReceived(pSmipMsg->Payload[1], (LonBool) pSmipMsg->Payload[2]);
                        break;
                        
                    case LonUsopGoUcfg:
                        LonGoUnconfiguredReceived();
                        break;
                        
                    case LonUsopGoCfg:
                        LonGoConfiguredReceived();
                        break;
                        
                    case LonUsopQueryAppSignature:
                    {
                        LonWord appSignature;
                        appSignature.msb = pSmipMsg->Payload[1];
                        appSignature.lsb = pSmipMsg->Payload[2];
                        LonAppSignatureReceived(appSignature);
                        break;
                    }
                    case LonUsopVersion:
                        LonVersionReceived(pSmipMsg->Payload[1], pSmipMsg->Payload[2], pSmipMsg->Payload[3],
                                           pSmipMsg->Payload[4], pSmipMsg->Payload[5], pSmipMsg->Payload[6]);
                        break;
                    case LonUsopEcho:
                        LonEchoReceived(&pSmipMsg->Payload[1]);
                        break;
                }
                break;
            #endif /* LON_UTILITY_FUNCTIONS */
            
            #if LON_ISI_ENABLED                
            case LonIsiNack:
                /* Received a Nack from the Micro Server regarding the Downlink Rpc.*/
                HandleDownlinkRpcAck((IsiRpcMessage*) pSmipMsg, FALSE);
                break;
                
            case LonIsiAck:
                /* Received an Ack from the Micro Server regarding the Downlink Rpc.*/
                HandleDownlinkRpcAck((IsiRpcMessage*) pSmipMsg, TRUE);
                break;
                
            case LonIsiCmd:
                /* Received an uplink Rpc from the Micro Server. */
                HandleUplinkRpc((IsiRpcMessage*) pSmipMsg);
                break;
            #endif /* LON_ISI_ENABLED */

            default:
            	break;
        }

        /* Release the receive buffer back to the serial driver. */
        LdvReleaseMsg(pSmipMsg);
    }
}

/*
 * Function: LonPollNv
 * Polls a bound, polling, input network variable.
 *
 * Parameters:
 * index - local index of the input network variable 
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks: 
 * Call this function to poll an input network variable. Polling an input network 
 * variable causes this device to solicit all devices that have output network 
 * variables connected to this input network variable to send their latest value 
 * for the corresponding network variable.
 *
 * The function returns LonApiNoError if the message is successfully put in the 
 * transmit queue to send. It is accompanied by the <LonNvUpdateCompleted> 
 * completion event.
 * Note the successful completion of this function does not indicate the successful 
 * arrival of the requested values. The values received in response to this poll 
 * are reported by a series of <LonNvUpdateOccurred> callback invocations. 
 *
 * LonPollNv operates on bound input network variables that have been declared 
 * with the Neuron C *polled* attribute, only. Also, only output network variables 
 * that are bound to the input network variable will be received.
 *
 * Note that it is *not* an error to poll an unbound polling input network 
 * variable.  If this is done, the application will not receive any 
 * LonNvUpdateOccurred() events, but will receive a LonNvUpdateCompleted() event 
 * with the �success� parameter set to TRUE. 
 */
const LonApiError LonPollNv(const unsigned nvIndex)
{
    LonApiError result = VerifyNv((LonByte) nvIndex, 0);
    
    if (result == LonApiNoError) 
    {
        LonSmipMsg* pSmipMsg = NULL;
        const LonNvDescription* const nvTable = LonGetNvTable();

        if (LON_GET_ATTRIBUTE(nvTable[nvIndex], LON_NVDESC_OUTPUT)) 
        {
            /* ...which must not be an output */
            result = LonApiNvPollOutputNv;
        } 
        else if (!LON_GET_ATTRIBUTE(nvTable[nvIndex], LON_NVDESC_POLLED)) 
        {
            /* ...that has been declared with the polled attribute */
            result = LonApiNvPollNotPolledNv;
        } 
        else if(LdvAllocateMsg(&pSmipMsg) != LonApiNoError) 
        {
            /* ...and if we have a buffer for this request */
            result = LonApiTxBufIsFull;
        } 
        else 
        {
            PrepareNvMessage(pSmipMsg, (LonByte) nvIndex, NULL, 0);
            LON_SET_ATTRIBUTE(NVMSG, LON_NVMSG_NVPOLL, 1);
            LdvPutMsg(pSmipMsg);
        }
    }
    return result;
}

/*
 * Function: LonPropagateNv
 * Propagates the value of a bound output network variable to the network.
 *
 * Parameters:
 * index - the local index of the network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function returns LonApiNoError if the outgoing NV-update message has been 
 * buffered by the driver, otherwise, an appropriate error code is returned.
 * See <LonNvUpdateCompleted> for the completion event that accompanies this API.
 */
const LonApiError LonPropagateNv(const unsigned nvIndex)
{
    LonApiError result = VerifyNv((LonByte) nvIndex, LonGetCurrentNvSize(nvIndex));

    if (result == LonApiNoError) 
    {
        const LonNvDescription* const nvTable = LonGetNvTable();
        if (!LON_GET_ATTRIBUTE(nvTable[nvIndex], LON_NVDESC_OUTPUT)) 
            /* can only propagate output network variables: */
            result = LonApiNvPropagateInputNv;     
        else
            result = SendNv((LonByte) nvIndex);
    }
    return result;
}

/*
 * Function: LonGetDeclaredNvSize
 * Gets the declared size of a network variable.
 *
 * Parameters:
 * index - the local index of the network variable
 *
 * Returns:     
 * Declared initial size of the network variable as defined in the Neuron C 
 * model file.
 * Zero if the network variable corresponding to index doesn't exist.
 *
 * Note that this function *could* be called from the LonGetCurrentNvSize() 
 * callback.
 */
const unsigned LonGetDeclaredNvSize(const unsigned nvIndex)
{
    unsigned returnSize = 0;
    LonApiError result = VerifyNv((LonByte) nvIndex, 0);

    if (result == LonApiNoError) 
    {
        const LonNvDescription* const pNvTable = LonGetNvTable();
        returnSize = pNvTable[nvIndex].DeclaredSize;
    }
    return returnSize;
}

/*
 * Function: LonGetNvValue
 * Returns a pointer to the network variable value.
 *
 * Parameters:
 * index - the index of the network variable
 *
 * Returns:
 * Pointer to the network variable value, or NULL if invalid.
 *
 * Remarks:
 * Use this function to obtain a pointer to the network variable value.  
 * Typically ShortStack applications use the global variable created by 
 * the LonTalk Interface Developer directly when accessing network variable 
 * values. This function can be used to obtain a pointer to the network variable 
 * value.
 */
volatile void* const LonGetNvValue(const unsigned nvIndex)
 {
    volatile void* p = NULL;
    LonApiError result = VerifyNv((LonByte) nvIndex, 0);

    if (result == LonApiNoError) 
    {
        const LonNvDescription* const pNvTable = LonGetNvTable();
        p = pNvTable[nvIndex].pData;
    }
    return p;
 }

/*
 * Function: LonSendResponse
 * Sends a response.
 *
 * Parameters:
 * correlator - message correlator obtained with <LonMsgArrived>
 * code - response message code
 * pData - pointer to response data, can be NULL iff len is zero
 * length - number of valid response data bytes in pData
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is called to send an explicit message response.  The correlator
 * is passed in to <LonMsgArrived> and must be copied and saved if the response 
 * is to be sent after returning from that routine.  A response code should be 
 * in the 0x00..0x2f range.
 */
const LonApiError LonSendResponse(const LonCorrelator correlator, 
    const LonByte code, const LonByte* const pData, const unsigned length)
{
    LonApiError result = LonApiNoError;
    LonSmipMsg* pSmipMsg = NULL;

    if (length > LON_MAX_MSG_DATA)
        /* Returns failure if the response data is too big */
        result = LonApiMsgLengthTooLong;
    else if ((LonServiceType) LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_SERVICE) != LonServiceRequest)
        /* Send response only if the incoming message has service type request/response */
        result = LonApiMsgNotRequest;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        /* Fail because of buffer shortage */
        result = LonApiTxBufIsFull;
    else 
    {
        /* OK, construct and post the response: */
    
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = (LonSmipCmd) (LonNiComm | 
                                    (LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_PRIORITY) ? 
                                     (LonByte) LonNiNonTxQueuePriority : (LonByte) LonNiNonTxQueue));
        pSmipMsg->Header.Length = (LonByte)(sizeof(LonExplicitMessage) - sizeof(EXPMSG.Data) + length);
    
        EXPMSG.Length = (LonByte)(length + 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_RESPONSE, 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_TAG));
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY, LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_PRIORITY));
        EXPMSG.Code = code;
    
        memcpy(&(EXPMSG.Data.Data), pData, length);
    
        LdvPutMsg(pSmipMsg);
    }
    return result;
}

/*
 * Function: LonGetUniqueId
 * Returns the unique ID (Neuron ID).
 *
 * Parameters:
 * pId   - pointer to the buffer to hold the unique ID
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to obtain the local Micro Server's unique ID, if available. 
 * The information might not be available immediately following a reset, prior to 
 * successful completion of the initialization sequence, or following an 
 * asynchronous reset of the host processor.
 *
 * See also the <LonGetLastResetNotification> function for alternative access 
 * to the same data.
 *
 * The *Unique ID* is also known as the *Neuron ID*, however, *Neuron ID* is a 
 * deprecated term. 
 *
 * Previously named lonGetNeuronId.
 */
const LonApiError LonGetUniqueId(LonUniqueId* const pNid)
{
    LonApiError result = LonApiNeuronIdNotAvailable;
    if (pNid != NULL && lastResetNotification.Version != 0xFF) 
    {
        memcpy(pNid, (void*) &(lastResetNotification.UniqueId), sizeof(LonUniqueId));
        result = LonApiNoError;
    }
    return result;  
}

/*
 * Function: LonGetVersion
 * Provides the link layer protocol version number.
 *
 * Parameters:
 * pVersion - pointer to hold the version number
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * The function provides the link layer protocol version number into the location
 * pointed to by *pVersion*, if available. 
 * The information may not be available immediately following a reset, prior 
 * to successful completion of the initialization sequence, or following 
 * an asynchronous reset of the host processor.
 *
 * See also the <LonGetLastResetNotification> function for alternative access 
 * to the same data.
 */
const LonApiError LonGetVersion(LonByte* const pVersion)
{
    LonApiError result = LonApiVersionNotAvailable;
    if (pVersion != NULL && lastResetNotification.Version != 0xFF) 
    {
        *pVersion = lastResetNotification.Version;     
        result = LonApiNoError;
    }
    return result;     /* TODO: Also do something about the supported max version number! As of ShortStack 2.10 this is no longer decorative */
}

/*
 * Function: LonSendServicePin
 * Propagates a service pin message.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to propagate a service pin message to the network. 
 * The function will fail if the device is not yet fully initialized.
 */
const LonApiError LonSendServicePin(void)
{
    return SendLocal(LonNiService, NULL, 0);
}

/*
 * Function: LonSendReset
 * Sends a reset message.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to send a reset message to the Micro Server. 
 * The function will fail if the device is not yet fully initialized.
 */
const LonApiError LonSendReset(void)
{
    return SendLocal(LonNiReset, NULL, 0);
}

/*
 * Function: LonGetLastResetNotification
 * Returns a pointer to the most recent reset notification, if any.
 *
 * Returns:
 * Pointer to <LonResetNotification>, reporting the most recent reset notification
 * buffered by the API. The returned pointer is NULL if such data is not available.
 *
 * Remarks:
 * The <LonReset> callback occurs when the Micro Server reports a reset, but the 
 * <LonResetNotification> data is buffered by the API for future reference. 
 * This is used by the API itself, but the application can query this buffer 
 * through the <LonGetLastResetNotification> function. This function delivers 
 * a superset of the information from the <LonGetUniqueId> and <LonGetVersion> 
 * functions.
 */
const volatile LonResetNotification* const LonGetLastResetNotification(void)
{
    return &lastResetNotification;
}

#if LON_APPLICATION_MESSAGES
/*
 * Function: LonSendMsg
 * Send an explicit (non-NV) message. 
 *
 * Parameters:
 * tag - message tag for this message
 * priority - priority attribute of the message
 * serviceType - service type for use with this message
 * authenticated - TRUE to use authenticated service
 * pDestAddr - pointer to destination address
 * code - message code
 * pData - message data, is NULL if length is zero
 * length - number of valid bytes available through pData
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is called to send an explicit message.  For application messages, 
 * the message code should be in the range of 0x00..0x2f.  Codes in the 0x30..0x3f 
 * range are reserved for protocols such as file transfer.
 *
 * If the tag field specifies one of the bindable messages tags (tag < #
 * bindable message tags), the pDestAddr is ignored (and can be NULL) because the 
 * message is sent using implicit addressing.
 *
 * A successful return from this function indicates only that the message has 
 * been queued to be sent.  If this function returns success, the ShortStack 
 * LonTalk Compact API will call <LonMsgCompleted> with an indication of the 
 * transmission success.
 *
 * If the message is a request, <LonResponseArrived> callback handlers are 
 * called when corresponding responses arrive.
 *
 * The device must be configured before your application calls this function.
 * If the device is unconfigured, the function will seem to 
 * work:  the application will receive a successful completion event (because 
 * the API will successfully pass the request to the Micro Server), but there 
 * will be no effect, and the application will not receive a callback (if any).
 */
const LonApiError LonSendMsg(const unsigned tag, const LonBool priority, const LonServiceType serviceType, 
            const LonBool authenticated, const LonSendAddress* const pDestAddr,
            const LonByte code, const LonByte* const pData, const unsigned length)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;
    //const LonMtDescription* const pMtTable = LonGetMtTable();
    LonSmipQueue queue;

    if (length > LON_MAX_MSG_DATA) 
        /* Returns failure if the message data is too big    */
        result = LonApiMsgLengthTooLong;
    else if ((LonMtCount == 0) || (tag > LonMtCount - 1) || (tag == NM_ND_TAG))
        /* LonMtCount contains the number of message tags    */
        /* declared in the Neuron C model file. tag          */
        /* must range from 0 to min(LonMtCount-1, ND_TAG-1). */
        result =  LonApiMsgInvalidMsgTag;
    #if    LON_EXPLICIT_ADDRESSING
    else if (pDestAddr == NULL /*&& pMtTable[tag]*/)
        /* Return failure if the message tag is associated with nonbind modifier   */
        /* and the explicit address is not present.                                */
        result = LonApiMsgExplicitAddrMissing;
    #endif /* LON_EXPLICIT_ADDRESSING */
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        queue = (serviceType == LonServiceUnacknowledged) ? (priority ? LonNiNonTxQueuePriority : LonNiNonTxQueue) :  
                                                            (priority ? LonNiTxQueuePriority : LonNiTxQueue);
        
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
        pSmipMsg->Header.Command = (LonSmipCmd) ((LonByte) LonNiComm | queue);
    
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, serviceType);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, tag);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, authenticated);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE, LonMessageExplicit);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY, priority);
        EXPMSG.Length = (LonByte)(length + 1);
        EXPMSG.Code = code;
    
        memcpy(EXPMSG.Data.Data, (void *)pData, length);
    
        #if    LON_EXPLICIT_ADDRESSING
        if (pDestAddr != NULL) 
        {
            memcpy(&(EXPMSG.Address), (void *)pDestAddr, sizeof(LonSendAddress));
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_EXPLICITADDR, 1);
        }
        #endif
    
        pSmipMsg->Header.Length = (LonByte) (sizeof(LonExplicitMessage) - sizeof(EXPMSG.Data) + length);    
        LdvPutMsg(pSmipMsg);
    }
    return result;
}
#endif    /* LON_APPLICATION_MESSAGES    */

#if LON_NM_QUERY_FUNCTIONS
/*
 * Function: LonQueryDomainConfig
 * Request a copy of a local domain table record.
 *
 * Parameters:
 * index - index of requested domain table record (0, 1)
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of a local domain table record. 
 * This is an asynchronous API. This function returns immediately.
 * The domain information will later be delivered to the <LonDomainConfigReceived>
 * callback.
 * This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryDomainConfig(const unsigned index)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;
    
    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (index > lastResetNotification.MaxDomains - 1u)
        result = LonApiIndexInvalid;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryDomainRequest));
    
        EXPMSG.Code = LonNmQueryDomain;
        EXPMSG.Length = (LonByte)(sizeof(LonNmQueryDomainRequest) + 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.QueryDomainRequest.Index = (LonByte) index;
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/*
 * Function: LonQueryNvConfig
 * Request a copy of network variable configuration data.
 *
 * Parameters:
 * index - index of requested NV configuration table entry
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's 
 * network variable configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonNvConfigReceived> 
 * callback. This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryNvConfig(const unsigned index)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;
    
    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else 
    {
        VerifyNv((LonByte) index, 0);
    
        if (result == LonApiNoError) 
        {
            if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
                result = LonApiTxBufIsFull;
            else 
            {
                memset(pSmipMsg, 0, sizeof(LonSmipMsg));
            
                pSmipMsg->Header.Command = LonNiNetManagement;
                EXPMSG.Code = LonNmQueryNvConfig;
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
                
                if (index < 255)
                {
                    pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonByte));
                    EXPMSG.Length = (LonByte)(sizeof(LonByte) + 1);
                    EXPMSG.Data.QueryNvAliasRequest.Index = (LonByte) index;
                }
                else
                {
                    pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryNvAliasRequest));
                    EXPMSG.Length = (LonByte)(sizeof(LonNmQueryNvAliasRequest) + 1);
                    EXPMSG.Data.QueryNvAliasRequest.Index = 255;
                    LON_SET_UNSIGNED_WORD(EXPMSG.Data.QueryNvAliasRequest.LongIndex, index);
                }
            
                LdvPutMsg(pSmipMsg);
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }
    return result;
}

/*
 * Function: LonQueryAliasConfig
 * Request a copy of alias configuration data.
 *
 * Parameters:
 * index - index of requested alias config table entry
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's 
 * alias configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonAliasConfigReceived> 
 * callback. This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryAliasConfig(const unsigned index)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;
    unsigned queryIndex = index + LonNvCount;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (index > lastResetNotification.MaxAliases - 1u)
        result = LonApiIndexInvalid;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
        
        pSmipMsg->Header.Command = LonNiNetManagement;
        EXPMSG.Code = LonNmQueryNvConfig;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        
        if (queryIndex < 255u)
        {
            pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonByte));
            EXPMSG.Length = (LonByte)(sizeof(LonByte) + 1);
            EXPMSG.Data.QueryNvAliasRequest.Index = (LonByte)queryIndex;
        }
        else
        {
            pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryNvAliasRequest));
            EXPMSG.Length = (LonByte)(sizeof(LonNmQueryNvAliasRequest) + 1);
            EXPMSG.Data.QueryNvAliasRequest.Index = 255u;
            LON_SET_UNSIGNED_WORD(EXPMSG.Data.QueryNvAliasRequest.LongIndex, queryIndex);
        }
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/*
 * Function: LonQueryAddressConfig
 * Request a copy of address table configuration data.
 *
 * Parameters:
 * index - index of requested address table entry
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's 
 * address table configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonAddressConfigReceived> 
 * callback. This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryAddressConfig(const unsigned index)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (index > lastResetNotification.MaxAddresses - 1u)
        result = LonApiIndexInvalid;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryAddressRequest));
    
        EXPMSG.Code = LonNmQueryAddr;
        EXPMSG.Length = (LonByte)(sizeof(LonNmQueryAddressRequest) + 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.QueryAddressRequest.Index = (LonByte) index;
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/*
 * Function: LonQueryConfigData
 * Request a copy of local configuration data.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's 
 * configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonConfigDataReceived> 
 * callback. This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryConfigData(void)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD + sizeof(LonNmReadMemoryRequest);
    
        EXPMSG.Code = LonNmReadMemory;
        EXPMSG.Length = (LonByte)(sizeof(LonNmReadMemoryRequest) + 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.ReadMemory.Mode = LonConfigStructRelative;
        EXPMSG.Data.ReadMemory.Count = (LonByte) sizeof(LonConfigData);
        LON_SET_UNSIGNED_WORD(EXPMSG.Data.ReadMemory.Address, 0);
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/*
 * Function: LonQueryStatus
 * Request local status and statistics.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to obtain the local status and statistics of the ShortStack 
 * device. This is an asynchronous API. This function returns immediately.
 * The data will later be delivered to the <LonStatusReceived> callback.
 * This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryStatus(void)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    { 
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD;
    
        EXPMSG.Code = LonNdQueryStatus;
        EXPMSG.Length = 1;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = ND_PENDING;
    }
    return result;
}

/*
 * Function: LonQueryTransceiverStatus
 * Request local transceiver status information.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to query the local transceiver status for the Micro Server. 
 * This is an asynchronous API. This function returns immediately.
 * The transceiver status will later be delivered to the <LonTransceiverStatusReceived> 
 * callback.
 * This function is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 *
 * This function works only for a Power Line transceiver. If your application
 * calls this function for any other transceiver type, the function will seem to 
 * work, but the corresponding callback handler will declare a failure.
 */
const LonApiError LonQueryTransceiverStatus(void)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD;
    
        EXPMSG.Code = LonNdQueryXcvr;
        EXPMSG.Length = 1;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);   /* Needed for node with NM authentication */

        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = ND_PENDING;
    }
    return result;
}
#endif /* LON_NM_QUERY_FUNCTIONS */

#if LON_NM_UPDATE_FUNCTIONS
/*
 * Function: LonSetNodeMode
 * Sets the ShortStack Micro Server's mode and state.
 *
 * Parameters:
 * mode - mode of the Micro Server, see <LonNodeMode>
 * state - state of the Micro Server, see <LonNodeState>
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to set the Micro Server's mode, state, or both. 
 * If the mode parameter is *LonChangeState*, then the state parameter can be 
 * set to either *LonConfigOffLine* or *LonConfigOnLine*.  Otherwise the state 
 * parameter should be *LonStateInvalid* (0).  Note that while the <LonNodeState> 
 * enumeration is used to report both the state and the mode (see <LonStatus>), 
 * it is *not* possible to change both the state and mode (online/offline) at 
 * the same time.
 * 
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 
 * You can also use the shorthand functions <LonGoOnline> and <LonGoOffline>.
 *
 * The device must be configured before your application calls this function.
 * If the device is unconfigured, the function will seem to work because the API 
 * will successfully pass the request to the Micro Server, but there will be no 
 * effect, and the application will not receive a callback (if any).
 */
const LonApiError LonSetNodeMode(LonNodeMode mode, LonNodeState state)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD + sizeof(LonNmSetNodeModeRequest);
    
        EXPMSG.Code = LonNmSetNodeMode;
        EXPMSG.Length = sizeof(LonNmSetNodeModeRequest) + 1;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.NodeMode.Mode = (LonNodeMode) mode;
        EXPMSG.Data.NodeMode.State = (LonNodeState) state;
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/* 
 * Function: LonUpdateAddressConfig
 * Updates an address table record on the Micro Server.
 *
 * Parameters:
 * index - address table index to update
 * pAddress - pointer to address table record 
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to write a record to the local address table.
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateAddressConfig(const unsigned index, const LonAddress* pAddress)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (index > lastResetNotification.MaxAddresses - 1u)
        result = LonApiIndexInvalid;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateAddressRequest);
    
        EXPMSG.Code = LonNmUpdateAddr;
        EXPMSG.Length = sizeof(LonNmUpdateAddressRequest) + 1;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.UpdateAddressRequest.Index = (LonByte) index;
        EXPMSG.Data.UpdateAddressRequest.Address = *pAddress;
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/* 
 * Function: LonUpdateAliasConfig
 * Updates a local alias table record.
 *
 * Parameters:
 * index - index of alias table record to update
 * pAlias - pointer to the alias table record
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function writes a record in the local alias table.
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateAliasConfig(const unsigned index, const LonAliasConfig* pAlias)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (index > lastResetNotification.MaxAliases - 1u)
        result = LonApiIndexInvalid;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result =  LonApiTxBufIsFull;
    else 
    {
        unsigned actualIndex = index + LonNvCount;
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateAliasRequest));
    
        EXPMSG.Code = LonNmUpdateNvConfig;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        
        if (actualIndex < 255)
        {
            /* Use the short form of the request */
            EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateAliasRequest*)0x0)->Request.ShortForm) + 1);
            EXPMSG.Data.UpdateAliasRequest.ShortIndex = (LonByte) actualIndex;  
            EXPMSG.Data.UpdateAliasRequest.Request.ShortForm.AliasConfig = *pAlias;
        }
        else
        {
            /* Use the long form of the request */
            EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateAliasRequest*)0x0)->Request.LongForm) + 1);
            EXPMSG.Data.UpdateAliasRequest.ShortIndex = 255;  
            LON_SET_UNSIGNED_WORD(EXPMSG.Data.UpdateAliasRequest.Request.LongForm.LongIndex, actualIndex);
            EXPMSG.Data.UpdateAliasRequest.Request.LongForm.AliasConfig = *pAlias;
        }
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/* 
 * Function: LonUpdateConfigData
 * Updates the configuration data on the Micro Server.
 *
 * Parameters:
 * pConfig - pointer to the <LonConfigData> configuration data
 * 
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to update the Micro Server's configuration data based on 
 * the configuration stored in the <LonConfigData> structure. 
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateConfigData(const LonConfigData* const pConfigData)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmWriteMemoryRequest) + sizeof(LonConfigData));
    
        EXPMSG.Code = LonNmWriteMemory;
        EXPMSG.Length = (LonByte) (sizeof(LonNmWriteMemoryRequest) + sizeof(LonConfigData) + 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.WriteMemory.Mode = LonConfigStructRelative;         /* Configuration address relative memory write */
        LON_SET_UNSIGNED_WORD(EXPMSG.Data.WriteMemory.Address, 0);
        EXPMSG.Data.WriteMemory.Count = (LonByte) sizeof(LonConfigData); 
        EXPMSG.Data.WriteMemory.Form = LonConfigCsRecalculationReset;   /* recalculate just configuration checksum     */
    
        memcpy((LonByte*)&(EXPMSG.Data) + sizeof(LonNmWriteMemoryRequest), pConfigData, sizeof(LonConfigData));
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/* 
 * Function:   LonUpdateDomainConfig
 * Updates a domain table record in the Micro Server.
 *
 * Parameters:
 * index - the index of the domain table to update
 * pDomain - pointer to the domain table record
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function can be used to update one record of the domain table.
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateDomainConfig(const unsigned index, const LonDomain* const pDomain)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (index > lastResetNotification.MaxDomains - 1u)
        result = LonApiIndexInvalid;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateDomainRequest));
    
        EXPMSG.Code = LonNmUpdateDomain;
        EXPMSG.Length = (LonByte)(sizeof(LonNmUpdateDomainRequest) + 1);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
        EXPMSG.Data.UpdateDomainRequest.Index = (LonByte)index;
        EXPMSG.Data.UpdateDomainRequest.Domain = *pDomain;
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = NM_PENDING;
    }
    return result;
}

/*
 * Function: LonUpdateNvConfig
 * Updates a network variable configuration table record in the Micro Server.
 *
 * Parameter:
 * index - index of network variable
 * pNvConfig - network variable configuration
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function can be used to update one record of the network variable
 * configuration table.
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateNvConfig(const unsigned index, const LonNvConfig* const pNvConfig)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;
    
    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else 
    {
        VerifyNv((LonByte) index, 0);

        if (result == LonApiNoError) 
        {
            if (CurrentNmNdStatus != NO_NM_ND_PENDING)
                result = LonApiNmNdAlreadyPending;
            else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
                result = LonApiTxBufIsFull;
            else 
            {
                memset(pSmipMsg, 0, sizeof(LonSmipMsg));
            
                pSmipMsg->Header.Command = LonNiNetManagement;
                pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateNvRequest));
                
                EXPMSG.Code = LonNmUpdateNvConfig;
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
                
                if (index < 255) /* index should always be less than 255 for this shortstack implementation as only a maximum of 254 nvs is possible*/
                {
                    /* Use the short form of the request */
                    EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateNvRequest*)0x0)->Request.ShortForm) + 1);
                    EXPMSG.Data.UpdateNvRequest.ShortIndex = (LonByte) index;  
                    EXPMSG.Data.UpdateNvRequest.Request.ShortForm.NvConfig = *pNvConfig;
                }
                else
                {
                    /* Use the long form of the request */
                    EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateAliasRequest*)0x0)->Request.LongForm) + 1);
                    EXPMSG.Data.UpdateNvRequest.ShortIndex = 255;  
                    LON_SET_UNSIGNED_WORD(EXPMSG.Data.UpdateNvRequest.Request.LongForm.LongIndex, index);
                    EXPMSG.Data.UpdateNvRequest.Request.LongForm.NvConfig = *pNvConfig;
                }
            
                LdvPutMsg(pSmipMsg);
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }
    return result;
}

/* 
 * Function: LonClearStatus
 * Clears the status statistics on the Micro Server.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function can be used to clear the Micro Server's status and statistics 
 * records.
 * This function is part of the optional network management update API 
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonClearStatus(void)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING)
        result = LonApiNmNdAlreadyPending;
    else if (LdvAllocateMsg(&pSmipMsg) != LonApiNoError)
        result = LonApiTxBufIsFull;
    else 
    {
        memset(pSmipMsg, 0, sizeof(LonSmipMsg));
    
        pSmipMsg->Header.Command = LonNiNetManagement;
        pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD;
    
        EXPMSG.Code = LonNdClearStatus;
        EXPMSG.Length = 1;
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
    
        LdvPutMsg(pSmipMsg);
        CurrentNmNdStatus = ND_PENDING;
    }    
    return result;
}
#endif    /* LON_NM_UPDATE_FUNCTIONS */

#if LON_UTILITY_FUNCTIONS
/*
 * Function: LonSendPing
 * Sends a ping command to the ShortStack Micro Server.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to send a ping command to the Micro Server 
 * to verify that communications with the Micro Server are functional.
 * The <LonPingReceived> callback handler function processes the reply to the query.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonSendPing(void)
{
    LonByte data[2];
    data[0] = LonUsopPing;
    data[1] = 0; /* Payload length should be at least two; so send a dummy byte. */
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonNvIsBound
 * Sends a command to the ShortStack Micro Server to query whether 
 * a given network variable is bound.
 *
 * Parameter:
 * index - index of network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to query whether the given network variable is bound. 
 * You can use this function to ensure that transactions are initiated only for
 * connected network variables.  The <LonNvIsBoundReceived> callback handler 
 * function processes the reply to the query.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonNvIsBound(const unsigned index)
{
    LonApiError result = VerifyNv((LonByte) index, 0);

    if (result == LonApiNoError) 
    {
        LonByte data[2];
        data[0] = LonUsopNvIsBound;
        data[1] = (LonByte) index;
        result = SendLocal(LonNiUsop, data, 2);
    }
    return result;  
}

/*
 * Function: LonMtIsBound
 * Sends a command to the ShortStack Micro Server to query whether 
 * a given message tag is bound.
 *
 * Parameter:
 * index - index of network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to query whether the given message tag is bound or not. 
 * You can use this function to ensure that transactions are initiated only for 
 * connected message tags.  The <LonMtIsBoundReceived> callback handler function
 * processes the reply to the query.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonMtIsBound(const unsigned index)
{
    LonApiError result = LonApiNoError;
    
    if ((LonMtCount == 0) || (index > LonMtCount - 1))
        result = LonApiIndexInvalid;
    else
    {
        LonByte data[2];
        data[0] = LonUsopMtIsBound;
        data[1] = (LonByte) index;
        result = SendLocal(LonNiUsop, data, 2);
    }
    return result;
}

/*
 * Function: LonGoUnconfigured
 * Puts the local node into the unconfigured state.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to put the Micro Server into the unconfigured state.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonGoUnconfigured(void)
{
    LonByte data[2];
    data[0] = LonUsopGoUcfg;
    data[1] = 0; /* Payload length should be at least two; so send a dummy byte. */
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonGoConfigured
 * Puts the local node into the configured state.
 *
 * Returns:
 * <LonApiError>.  
 *
 * Remarks:
 * Call this function to put the Micro Server in the configured state and online 
 * mode.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonGoConfigured(void)
{
    LonByte data[2];
    data[0] = LonUsopGoCfg;
    data[1] = 0; /* Payload length should be at least two; so send a dummy byte. */
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonQueryAppSignature
 * Queries the Micro Server's current version of the host application signature.
 *
 * Parameter:
 * bInvalidate - flag to indicate whether to invalidate the signature
 * 
 * Returns:
 * <LonApiError>.  
 *
 * Remarks:
 * Call this function to query the Micro Server's current version of the host 
 * application signature. If the bInvalidate flag is TRUE, the Micro Server 
 * invalidates its copy of the signature *AFTER* reporting it to the host.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonQueryAppSignature(LonBool bInvalidate)
{
    LonByte data[2];
    data[0] = LonUsopQueryAppSignature;
    data[1] = bInvalidate ? 1 : 0;
    return SendLocal(LonNiUsop, data, 2);
}

/* 
 * Function: LonQueryVersion 
 * Request version details from the Micro Server 
 *  
 * Returns: 
 * <LonApiError>. 
 *  
 * Remarks: 
 * Call this function to request the Micro Server application and core library 
 * version numbers, both delivered as triplets of major version, minor version 
 * and build number (one byte each) through the <LonVersionReceived>() callback. 
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonQueryVersion() 
{
    const LonByte data[2] = { LonUsopVersion, 0 };
    return SendLocal(LonNiUsop, data, 2);
}

/* 
 * Function: LonRequestEcho
 * Sends arbitrary test data to the Micro Server and requests a modulated echo.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks: 
 * LonRequestEcho transmits LON_ECHO_SIZE (6) bytes of arbitrary data, chosen by
 * the caller of the function, to the Micro Server. The Micro Server returns these
 * data bytes to the host, but increments each byte by one, using unsigned 8-bit
 * addition without overflow. When the response is received, the API activates the
 * <LonEchoReceived> callback. 
 */ 
const LonApiError LonRequestEcho(const LonByte data[LON_ECHO_SIZE])
{
    LonByte payload[1+LON_ECHO_SIZE] = { LonUsopEcho };
    memcpy(&payload[1], data, LON_ECHO_SIZE);
    return SendLocal(LonNiUsop, payload, 1+LON_ECHO_SIZE);
}

#endif /* LON_UTILITY_FUNCTIONS */
