/*
 * Filename: ShortStackHandlers.c
 *
 * Description: This file contains callback handler functions that must be
 * implemented by the application.
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

#include "ShortStackDev.h"
#include "ShortStackApi.h"
#if LON_DMF_ENABLED
#include "string.h"         /* Required for memcpy */
#endif /* LON_DMF_ENABLED */

/* 
 * The actual implementations of the callback handlers that this example needs
 * are available in Main.c. Here are the forward declarations for those functions. 
 */
extern void myNvUpdateOccurred(const LonByte nvIndex, const LonReceiveAddress* const pNvInAddr);
extern void myResetOccurred(const LonResetNotification* const pResetNotification);
extern void myWinkOccurred();
extern void myDomain(const LonDomain* const pDomain);

/* 
 * Callback:   LonResetOccurred
 * Occurs when the Micro Server has completed a reset.
 *
 * Parameters:
 * pResetNotification - <LonResetNotification> structure with capabilities and 
 * identifying data.
 *
 * Remarks:
 * The pointer to <LonResetNotification> is only valid for the duration of 
 * this callback, but the API buffers the notification details. Use the 
 * <LonGetLastResetNotification> function to fetch a pointer to that buffer when 
 * needed.
 *
 * The reset message contains the link-layer protocol version number implemented 
 * by the Micro Server. The application should make sure that it can handle that 
 * link-layer protocol by comparing it with the API's link-layer protocol 
 * version number defined by LON_LINK_LAYER_PROTOCOL_VERSION.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonResetOccurred(const LonResetNotification* const pResetNotification)
{
    myResetOccurred(pResetNotification);
}
#endif

/*
 * Callback: LonWink
 * Occurs when the Micro Server has received a WINK command.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonWink(void)
{
	myWinkOccurred();
}
#endif

/*
 * Callback: LonOffline
 * Occurs when the Micro Server has received a command to enter the offline state.
 *
 * Remarks:
 * The Micro Server does not actually go offline until the <LonOffline> callback 
 * function returns and the ShortStack LonTalk Compact API sends a confirmation 
 * message to the Micro Server. Thus, the host application should assume that 
 * the Micro Server is offline only after this callback completes processing.
 */
//#ifndef LON_FRAMEWORK_TYPE_III
void LonOffline(void)
{
    /* 
     * TO DO
     */
}
//#endif
/*
 * Callback: LonOnline
 * Occurs when the Micro Server has received a command to enter the online state.
 *
 * Remarks:  
 * The Micro Server does not actually go online until the <LonOnline> callback 
 * function returns and the ShortStack LonTalk Compact API sends a confirmation 
 * message to the Micro Server. Thus, the host application should assume that 
 * the Micro Server is online only after this callback completes processing.
 */
//#ifndef LON_FRAMEWORK_TYPE_III
void LonOnline(void)
{
    /* 
     * TO DO
     */
}
//#endif
/*
 * Callback: LonServicePinPressed
 * Occurs when the service pin has been activated.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonServicePinPressed(void)
{
    /* 
     * TO DO
     */
}
#endif

/*
 * Callback: LonServicePinHeld
 * Occurs when the service pin has been continuously activated for a 
 * configurable time.
 *
 * Remarks:
 * Use the LonTalk Interface Developer to enable this feature and to specify the 
 * duration for which the service pin must be activated to trigger this callback. 
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonServicePinHeld(void)
{
    /*
     * TO DO
     */
}
#endif
/*
 * Callback: LonNvUpdateOccurred
 * Occurs when new input network variable data has arrived.
 *
 * Parameters:
 * index - global index (local to the device) of the network variable in question 
 * pSourceaddress - pointer to source address description 
 *
 * Remarks:
 * The network variable with local index given in this callback has been updated 
 * with a new value. The new value is already stored in the network variable's 
 * location; access the value through the global variable representing the 
 * network variable, or obtain the pointer to the network variable's value from 
 * the network variable table. The pSourceAddress pointer is only valid for the 
 * duration of this callback. 
 *
 * For an element of a network variable array, the index is the global network 
 * variable index plus the array-element index. For example, if nviVolt[0] has
 * global network variable index 4, then nviVolt[1] has global network variable 
 * index 5.
 *
 * Also, if this is a non-volatile network variable, that is, it has the 
 * LON_NVDESC_PERSISTENT_MASK attribute set, store its value in non-volatile 
 * memory as implemented by your application. An network variable is a 
 * non-volatile network variable if it is declared with an *eeprom* keyword in 
 * the model file or if it is a configuration property network variable (CPNV).
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonNvUpdateOccurred(const unsigned index, const LonReceiveAddress* const pSourceAddress)
{
    // myNvUpdateOccurred(index, pSourceAddress);
}
#endif

/*
 * Callback:   LonNvUpdateCompleted
 * Signals completion of a network variable update
 *
 * Parameters:
 * index - global index (local to the device) of the network variable that was 
 * processed
 * success - indicates whether the update was successful or unsuccessful
 *
 * Remarks:
 * This callback completes the transaction that was started by calling the 
 * <LonPropagateNv> or <LonPollNv> API functions.  The index parameter 
 * delivered with this callback matches the one from the API invocation.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonNvUpdateCompleted(const unsigned index, const LonBool success)
{
    /* 
     * TO DO
     */
}
#endif

/*
 * Callback: LonGetCurrentNvSize
 * Gets the current size of a network variable.
 *
 * Parameters:
 * index - the local index of the network variable
 *
 * Returns:    Current size of the network variable.
 *             Zero if the network variable corresponding to index doesn't exist.
 *
 * Remarks:
 * If the network variable size is fixed, this function should return 
 * <LonGetDeclaredNvSize>. If the network variable size is changeable, the 
 * current size should be returned. The default implementation for changeable 
 * type network variables returns 0, and must be updated by the application 
 * developer.  
 *
 * The Micro Server will not propagate a network variable with size 0, nor will 
 * it generate an update event if a network variable update is received from the 
 * network when the current network variable size is 0.
 *
 * Note that even though this is a callback function, it *is* legal for the 
 * application to call <LonGetDeclaredNvSize> from this callback.
 */
//#ifndef LON_FRAMEWORK_TYPE_III
const unsigned LonGetCurrentNvSize(const unsigned nvIndex)
{
    unsigned size = 0;
    const LonNvDescription* const pNvTable = LonGetNvTable();

    if (LON_GET_ATTRIBUTE(pNvTable[nvIndex], LON_NVDESC_CHANGEABLE)) 
    {
        /* 
         * TO DO: Supply the current size of this network variable.
         * The current size can typically be found in the associated
         * SCPTnvType configuration property. 
         * Note the NvTable holds the network variable's declared
         * size, which equals its maximum size, but not necessarily
         * its current size.
         */



    } 
    else 
    {
        /* 
         * For not changeable network variables, the declared 
         * size held in the table also equals the current size.
         */
        size = LonGetDeclaredNvSize(nvIndex);
    }
    return size;
}
//#endif
/*
 * Callback: LonNvdDeserializeNvs
 * Initializes the network variable values from non-volatile storage.
 *
 * Remarks:
 * This callback is called by the API's initialization routine <LonInit>.
 * If the device contains any network variable that requires persistent storage, 
 * such as an eeprom network variable or a configuration property network variable 
 * (CPNV), the application should provide a mechanism to store and retrieve its 
 * data from persistent memory.
 */
const LonApiError LonNvdDeserializeNvs(void)
{
#if LON_PERSISTENT_NVS
    /*
     * TO DO:
     * This application implements eeprom network variables and/or configuration network variables.
     * You must write code that loads these variables' data from non-volatile data storage. 
     * Return LonApiNoError upon successful completion, or return LonApiInitializationFailure if 
     * data retrieval fails. 
     * You can iterate the LID-generated network variable table to identify those network variables; 
     * look for the LON_NVDESC_PERSISTENT_MASK attribute. For changeable-type network variables, 
     * always process data of the declared size. You can obtain the declared size from the network 
     * variable table, or use the sizeof() operator on the associated global variable.
     *
     * Note that you must also add code to the <LonNvUpdateOccurred> callback handler to identify
     * an update to one of these network variables, and to update the non-volatile storage with the
     * most recent value.
     */
 
 
 
    return LonApiInitializationFailure;
#else
    /*
     * This application implements no eeprom network variables or configuration network variables.
     * No change to this code is needed
     */
    return LonApiNoError;
#endif  /* LON_PERSISTENT_NVS */
}

#if LON_APPLICATION_MESSAGES

/*
 * Callback: LonMsgArrived
 * Occurs when an application message arrives.
 *
 * Parameters:
 * pAddress - source and destination address (see <LonReceiveAddress>)
 * correlator - correlator; preserve and use with <LonSendResponse>
 * priority - indicates whether its a priority message 
 * serviceType - indicates the service type of the message
 * authenticated - TRUE if the message was (successfully) authenticated
 * code - message code
 * pData - pointer to message data bytes, might be NULL if dataLength is zero
 * dataLength - length of bytes pointed to by pData
 *
 * Remarks:
 * This callback reports the arrival of a message that is neither a network 
 * variable message, nor a non-Nv message that is otherwise processed by the 
 * Micro Server. Typically, this callback handler function is used with 
 * application message codes in the value range indicated by the 
 * <LonApplicationMessageCode> enumeration.
 * All pointers are only valid for the duration of this callback. 
 */

extern void myLonMsgArrived();

void LonMsgArrived(const LonReceiveAddress* const pAddress, const LonCorrelator correlator, 
                   const LonBool priority, const LonServiceType serviceType, const LonBool authenticated, 
                   const LonByte code, const LonByte* const pData, const unsigned dataLength)
{
	myLonMsgArrived(pData, dataLength);
}

/*
 * Callback: LonResponseArrived
 * Occurs when a response arrived.
 *
 * Parameters:
 * pAddress - source and destination address used for response (see <LonResponseAddress>)
 * tag - tag to match the response to the request
 * code - response code
 * pData - pointer to response data, might by NULL if dataLength is zero
 * dataLength = number of bytes available through pData.
 *
 * Remarks:
 * This callback occurs when a message arrives in response to an earlier request, 
 * sent with the <LonSendMsg> API.
 */
void LonResponseArrived(const LonResponseAddress* const pAddress, const unsigned tag, const LonByte code, 
                        const LonByte* const pData, const unsigned dataLength)
{
    /*
     * TO DO
     */
}

/* 
 * Callback: LonMsgCompleted
 * Occurs when a message transaction has completed.  See <LonSendMsg>.
 *
 * Parameters:
 * tag - use to correlate the event with the message sent
 * success - TRUE for successful completion, otherwise FALSE
 *
 * Remarks:
 * For unacknowledged or repeated messages, the transaction is complete when 
 * the message has been sent with the configured number of retries. For 
 * acknowledged messages, the Micro Server calls <LonMsgCompleted> with *success* 
 * set to TRUE after receiving acknowledgments from all of the destination 
 * devices, and calls <LonMsgCompleted> with *success* set to FALSE if
 * the transaction timeout period expires before receiving acknowledgements  
 * from all destinations.  Likewise, for request messages, the transaction is 
 * considered successful when the Micro Server receives a response from each of the 
 * destination devices, and unsuccessful if the transaction timeout expires 
 * before responses have been received from all destinations devices.
 */
void LonMsgCompleted(const unsigned tag, const LonBool success)
{
    /* 
     * TO DO
     */
	// ed2dani - I get here when I send my dummy 'test' message... I am not sure what to do next.
	// ed2dani2 - I get here when I send a _good_ test message.
}

#endif    /* LON_APPLICATION_MESSAGES    */


#if LON_NM_QUERY_FUNCTIONS    /* used to be extended API callbacks */

/*
 * Callback: LonDomainConfigReceived
 * Callback following call to <LonQueryDomainConfig>.
 *
 * Parameters:
 * pDomain - pointer to the <LonDomain> structure iff success is TRUE
 * success - boolean success indicator
 *
 * Remarks:
 * The pDomain pointer to the <LonDomain> structure is only valid for the 
 * duration of this callback, and only if the success parameter indicates 
 * success. Applications that need to preserve this information beyond the 
 * duration of this callback must make a copy of the <LonDomain> data. 
 * This callback is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
void LonDomainConfigReceived(const LonDomain* const pDomain, const LonBool success)
{
    if (success)
    	myDomain(pDomain);
}

/*
 * Callback: LonNvConfigReceived
 * Callback following call to <LonQueryNvConfig>.
 *
 * Parameters:
 * pNvConfig - pointer to the <LonNvConfig> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pNvconfig pointer is only valid for the duration of this callback, and 
 * only if the success parameter is true. Applications that need to preserve this 
 * information beyond the duration of this callback must make a copy of the 
 * <LonNvConfig> data. This callback is part of the optional 
 * network management query API (LON_NM_QUERY_FUNCTIONS).
 */
void LonNvConfigReceived(const LonNvConfig* const pNvConfig, const LonBool success)
{
    /* 
     * TO DO
     */
}

/*
 * Callback: LonAliasConfigReceived
 * Callback following call to <LonQueryAliasConfig>.
 *
 * Parameters:
 * pAliasConfig - pointer to the <LonAliasConfig> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pAliasConfig pointer is only valid for the duration of this callback, and 
 * only if the success parameter is true. Applications that need to preserve this 
 * information beyond the duration of this callback must make a copy of the 
 * <LonAliasConfig> data.
 * This callback is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
void LonAliasConfigReceived(const LonAliasConfig* const pAliasConfig, const LonBool success)
{
    /* 
     * TO DO
     */
}

/*
 * Callback:   LonAddressConfigReceived
 * Callback following call to <LonQueryAddressConfig>.
 *
 * Parameters:
 * pAddress - pointer to the <LonAddress> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pAddress pointer is only valid for the duration of this callback, and 
 * only if the success parameter is true. Applications that need to preserve this 
 * information beyond the duration of this callback must make a copy of the 
 * <LonAddress> data. This callback is part of the optional network management 
 * query API (LON_NM_QUERY_FUNCTIONS).
 */
void LonAddressConfigReceived(const LonAddress* const pAddress, const LonBool success)
{
    /* 
     * TO DO
     */
}

/*
 * Callback: LonConfigDataReceived
 * Callback following call to <LonQueryConfigData>.
 *
 * Parameters:
 * pConfigData - pointer to the <LonConfigData> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pConfigData pointer is only valid for the duration of this callback, 
 * and only if the success parameter is true. Applications that need to preserve 
 * this information beyond the duration of this callback must make a copy of 
 * the <LonConfigData> data.
 * This callback is part of the optional network management query API 
 * (LON_NM_QUERY_FUNCTIONS).
 */
void LonConfigDataReceived(const LonConfigData* const pConfigData, const LonBool success)
{
    /* 
     * TO DO
     */
}

/*
 * Callback: LonStatusReceived
 * Callback following call to <LonQueryStatus>.
 *
 * Parameters:
 * pStatus - pointer to the <LonStatus> structure iff successful.
 * success - boolean success indicator
 *
 * Remarks:
 * The pStatus pointer is only valid for the duration of this callback, and only 
 * if the success parameter is true. Applications that need to preserve this 
 * information beyond the duration of this callback must make a copy of the 
 * <LonStatus> data. This callback is part of the optional network management 
 * query API (LON_NM_QUERY_FUNCTIONS).
 */
void LonStatusReceived(const LonStatus* const pStatus, const LonBool success)
{
    /* 
     * TO DO
     */
}

/*
 * Callback: LonTransceiverStatusReceived
 * Callback following a call to <LonQueryTransceiverStatus>.
 *
 * Parameters:
 * pStatus - pointer to the <LonTransceiverParameters> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pStatus pointer is only valid for the duration of this callback, and 
 * only if the success parameter is true. Applications that need to preserve this 
 * information beyond the duration of this callback must make a copy of the 
 * <LonTransceiverParameters> data. This callback is part of the optional 
 * network management query API (LON_NM_QUERY_FUNCTIONS).
 *
 * The function <LonQueryTransceiverStatus> only works for a Power Line 
 * transceiver. If your application invokes that function for any other 
 * transceiver type, this callback will declare a failure through the second 
 * parameter.
 */
void LonTransceiverStatusReceived(const LonTransceiverParameters* const pStatus, const LonBool success)
{
    /* 
     * TO DO
     */
}

#endif /* LON_NM_QUERY_FUNCTIONS */

#if LON_DMF_ENABLED

/* 
 * Callback: LonMemoryRead
 * Read memory in the ShortStack device's memory space.
 *
 * Parameters:
 * address - virtual address of the memory to be read
 * size - number of bytes to read
 * pData - pointer to a buffer to store the requested data
 *
 * Remarks:
 * The ShortStack event handler calls <LonMemoryRead> whenever it receives a 
 * network management memory read request that fits into the registered file 
 * access window. This callback function is used to read data starting at the 
 * specified virtual Smart Transceiver memory. This applies to template files, 
 * configuration property value files, user-defined files, and possibly to other 
 * data. The address space for this command is limited to the Smart 
 * Transceiver�s 64 KB address space.
 */
const LonApiError LonMemoryRead(const unsigned address, const unsigned size, void* const pData) 
{
    char* pHostAddress = NULL;
    LonMemoryDriver driver = LonMemoryDriverUnknown;
    LonApiError result = LonTranslateWindowArea(FALSE, address, size, &pHostAddress, &driver);
 
    if (result == LonApiNoError) 
    {
        if (driver == LonMemoryDriverStandard) 
        {
            (void) memcpy(pData, pHostAddress, size);
        } 
        else 
        {
            /*
             * TO DO: add code to support alternative data storage, 
             * such as using paged memory, or serial interface memory 
             * devices such as IIC EEPROM devices. When completing  
             * with success, return LonApiNoError.
             */
 
 
 
            result = LonApiDmfNoDriver;
        }
    }
    return result;
}
 
/* 
 * Callback: LonMemoryWrite
 * Update memory in the ShortStack device's memory space.
 *
 * Parameters:
 * address - virtual address of the memory to be update
 * size - number of bytes to write
 * pData - pointer to the data to write
 *
 * Remarks:
 * The ShortStack event handler calls <LonMemoryWrite> whenever it receives a 
 * network management memory write request that fits into the registered file 
 * access window. This callback function is used to write data at the specified 
 * virtual Smart Transceiver memory.  This applies to configuration property 
 * value files, user-defined files, and possibly to other data. The address space 
 * for this command is limited to the Smart Transceiver�s 64 KB address space.
 */
const LonApiError LonMemoryWrite(const unsigned address, const unsigned size, const void* const pData) 
{
    char* pHostAddress = NULL;
    LonMemoryDriver driver = LonMemoryDriverUnknown;
    LonApiError result = LonTranslateWindowArea(TRUE, address, size, &pHostAddress, &driver);
 
 
    if (result == LonApiNoError) 
    {
        if (driver == LonMemoryDriverStandard) 
        {
            (void) memcpy(pHostAddress, pData, size);
        }
        else
        {
            /*
             * TO DO: add code to support alternative data storage, 
             * such as using paged memory, or serial interface memory 
             * devices such as IIC EEPROM devices. When completing  
             * with success, return LonApiNoError.
             */
 
 
 
            result = LonApiDmfNoDriver;
        }
    }
    return result;
}

#endif  /* LON_DMF_ENABLED */


#if LON_UTILITY_FUNCTIONS
/* 
 * Callback: LonPingReceived
 * The ShortStack Micro Server has sent a ping command.
 *
 * Remarks:
 * The ShortStack Micro Server has sent a ping command. This command can be
 * a response to the ping command sent by the host application
 * to the Micro Server. This callback is part of the optional 
 * utility API (LON_UTILITY_FUNCTIONS).
 */
void LonPingReceived(void)
{
    /* 
     * TO DO
     */
}

/* 
 * Callback: LonNvIsBoundReceived
 * Callback following a call to <LonNvIsBound>.
 *
 * Parameters:
 * index - index of the network variable
 * bound - indicates whether the network variable is bound 
 *
 * Remarks:
 * The Micro Server has responded to the <LonNvIsBound> request. The boolean
 * variable *bound* tells whether the network variable identified by index
 * is bound. This callback is part of the optional 
 * utility API (LON_UTILITY_FUNCTIONS).
 */
void LonNvIsBoundReceived(const unsigned index, const LonBool bound)
{
    /* 
     * TO DO
     */
}

/* 
 * Callback: LonMtIsBoundReceived
 * Callback following a call to <LonMtIsBound>.
 *
 * Parameters:
 * index - index of the message tag
 * bound - indicates whether the message tag is bound
 *
 * Remarks:
 * The Micro Server has responded to the <LonMtIsBound> request. The boolean
 * variable *bound* tells whether the message tag identified by index
 * is bound. This callback is part of the optional 
 * utility API (LON_UTILITY_FUNCTIONS).
 */
void LonMtIsBoundReceived(const unsigned index, const LonBool bound)
{
    /* 
     * TO DO
     */
}

/* 
 * Callback: LonGoUnconfiguredReceived
 * Callback following a call to <LonGoUnconfigured>.
 *
 * Remarks:
 * The Micro Server has responded to the <LonGoUnconfigured> request. 
 * If the Micro Server was in a configured state before the <LonGoUnconfigured> 
 * request was sent, it will reset after going to the unconfigured state. 
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
void LonGoUnconfiguredReceived(void)
{
    /* 
     * TO DO
     */
}

/* 
 * Callback: LonGoConfiguredReceived
 * Callback following a call to <LonGoConfigured>.
 *
 * Remarks:
 * The Micro Server has responded to the <LonGoConfigured> request. 
 * The Micro Server will not reset after going into the configured state unless
 * some serious error, such as an application checksum error, is detected in the
 * process.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
void LonGoConfiguredReceived(void)
{
    /* 
     * TO DO
     */
}

/*
 * Callback: LonAppSignatureReceived
 * Callback following a call to <LonQueryAppSignature>.
 *
 * Parameter:
 * appSignature - Micro Server's copy of the host's application signature
 * 
 * Remarks:
 * The Micro Server has responded to the <LonQueryAppSignature> request. 
 * If the *bInvalidate* flag was set to TRUE in the <LonQueryAppSignature> 
 * request, the Micro Server has already invalidated the signature by the time 
 * this callback is called. 
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
void LonAppSignatureReceived(LonWord appSignature)
{
    /* 
     * TO DO
     */
}

/* 
 * Callback: LonVersionReceived
 * Callback following a call to <LonQueryVersion>.
 *
 * Parameters:
 * appMajor - the major version number for the Micro Server application, 0..255
 * appMinor - the minor version number for the Micro Server application, 0..255
 * appBuild - the build number for the Micro Server application, 0..255
 * coreMajor - the major version number for the Micro Server core library, 0..255
 * coreMinor - the minor version number for the Micro Server core library, 0..255
 * coreBuild - the build number for the Micro Server core library, 0..255
 *
 * Remarks:
 * The Micro Server has responded to a <LonQueryVersion> request. 
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
*/ 
void LonVersionReceived(unsigned appMajor, unsigned appMinor, unsigned appBuild, 
                        unsigned coreMajor, unsigned coreMinor, unsigned coreBuild) 
{
    /* 
     * TO DO
     */
}

/* 
 * Callback: LonEchoReceived
 * Callback following a call to <LonRequestEcho>.
 *
 * Parameters:
 * LON_ECHO_SIZE payload bytes, as sent through <LonRequestEcho>, but modified by the
 * Micro Server by adding one to each byte, using an unsigned 8-bit addition without
 * overflow detection.
 *
 * Remarks:
 * The Micro Server has responded to a <LonRequestEcho> request. The host application
 * should now confirm that the echoed data matches the expectations; mismatching data
 * can indicate an incorrect host application or link layer driver, or permanent or
 * transient link layer errors. These link layer errors could, for example, be
 * introduced by incorrect line termination, excessive coupling or cross-talk, or by
 * excessive or out-of-sync link layer bit rates.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
void LonEchoReceived(const LonByte data[LON_ECHO_SIZE]) 
{
    /* 
     * TO DO
     */
}

#endif  /* LON_UTILITY_FUNCTIONS */

