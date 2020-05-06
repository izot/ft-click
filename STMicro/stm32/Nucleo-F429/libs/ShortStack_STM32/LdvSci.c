/*
 *  Filename: LdvSci.c
 *
 *  Description:  This file contains the ARM7 ShortStack SCI driver 
 *  implementation to interface with a ShortStack Micro Server.
 *
 * Copyright (c) Echelon Corporation 2008.  All rights reserved.
 *
 * This file is Example Software as defined in the Software
 * License Agreement that governs its use.
 *
 * ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
 * ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
 * ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
 * QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE, 
 * NONINFRINGEMENT, AND THEIR EQUIVALENTS.
 */

#include "LonPlatform.h"
#include "LdvSci.h"
#include "main.h"

#if defined(STM32F103xB)
	#include "stm32f1xx_hal.h"
	#define USART_RX_DATA_REGISTER	DR
	#define USART_TX_DATA_REGISTER	DR
	#define USART_STATUS_REGISTER	SR
#elif defined(STM32F429xx)
	#include "stm32f4xx_hal.h"
	#include "core_cm4.h"
	#define USART_RX_DATA_REGISTER	DR
	#define USART_TX_DATA_REGISTER	DR
	#define USART_STATUS_REGISTER	SR
#elif defined(STM32G071xx)
	#include "stm32g0xx_hal.h"
	#include "stm32g071xx.h"
	#define USART_RX_DATA_REGISTER	RDR
	#define USART_TX_DATA_REGISTER	TDR
	#define USART_STATUS_REGISTER	ISR  // USART Interrupt and status register
	#define USART_SR_RXNE_Msk	USART_ISR_RXNE_RXFNE_Msk
	#define USART_SR_TXE_Msk USART_ISR_TXE_TXFNF_Msk
#else
	#error "Not supported platform."
#endif


#include "cmsis_gcc.h"
#include "cmsis_os.h"
#include "main.h"

#ifdef PRINT_LINK_LAYER
#include "stdio.h"
#include "string.h"
#endif

#include "ShortStackSupport.h"


/* 
 * Define number of ticks for the periodic interval timer.
 * 1 ms is approximately 3000 ticks for a clock running at 48 MHz
 */
#define PIV_1_MS                        3000

LdvSysTxBuffer              TxBuffer[LDV_TXBUFCOUNT];
LdvSysRxBuffer              RxBuffer[LDV_RXBUFCOUNT];
volatile LdvDriverStatus    DriverStatus;

extern UART_HandleTypeDef FT_UART;

LonBool neuron_txint = FALSE;
LonBool neuron_rxint = FALSE;

/* counters for debug purposes */
LonUbits32 nRxErrors = 0;
LonUbits32 nRxTimeout = 0;
LonUbits32 nTxBufUnavailable = 0;

/* Utility Timers */
unsigned nUtilityUpTimer[4] = {0}; 		/* 4 utility timers that count up */
unsigned nUtilityDownTimer[4] = {0}; 	/* 4 utility timers that count down */
#define MAX_UNSIGNED    0xFFFFFFFF

/* 
 * Intrinsic functions used to enable and disable global interrupts.
 * The interrupts should be disabled before accessing any shared date,
 * and enabled afterwards.
 */
#define EnableGlobalInterrupts()   __enable_irq()
#define DisableGlobalInterrupts()  __disable_irq()

/*
 * Forward declarations for the interrupt handler functions.
*/
extern void RxTxInterruptHandler(void);
extern void PeriodicIntervalTimerHandler(void);

/*
 * Internal function used to reset the Micro Server.
 */
static void ResetMicroServer(void) 
{
	ASSERT_RESET();
	SleepMs(50);
	DEASSERT_RESET(); // Must be configured as open drain, with pullup resistor
	SleepMs(200);
}

/*
 *  Function: SuspendSci
 *  Function to suspend the SCI driver.
 */
LonBool SuspendSci(void)
{
    /* Make sure that the transmitter and receiver are idle.
     * Also, make sure we haven't asserted the RTS line to begin any new transmission. */
    if (DriverStatus.TxState == LdvTxIdle  &&  DriverStatus.RxState == LdvRxIdle  &&  CHECK_RTS_DEASSERTED())
    {
        /* Deassert the HRDY to tell the mircoserver to stop sending data. */
        DEASSERT_HRDY();
        /* Wait for the time it takes to transfer 2 bytes of packet header (approximately 2 ms).
         * The Micro Server may have started sending the packet before seeing the HRDY deasserted. */
        SleepMs(2);
        /* Check if the receiver is still idle */
        if (DriverStatus.RxState == LdvRxIdle)
        {
            /* Receiver is idle. We can safely suspend the driver */
            DISABLE_TX_INT();
            DISABLE_RX_INT();
            DriverStatus.DriverState = LdvDriverSleep;       /* Put the driver into sleep mode */
            DEASSERT_RTS();
            return TRUE;
        }
        /* Else receiver is not idle.
         * Don't suspend the driver, but don't reassert the HRDY
         * so that it can be suspended the next time.
         * If it doesn't need to be suspended, the HRDY will automatically
         * get asserted when there is an empty receive buffer */
    }
    return FALSE;
}


/*
 *  Function: ResumeSci
 *  Function to resume the SCI driver.
 */
void ResumeSci(void)
{
    /* Make sure that the driver was actually suspended
     * before resuming it */
    if (DriverStatus.DriverState == LdvDriverSleep)
    {
        unsigned int i;
        
        DriverStatus.DriverState = LdvDriverNormal;
        /* Clear interrupts if any, by reading the status register */
        ENABLE_RX_INT();        /* Enable receive interrupt */
        /* Resume Neuron if receive buffer is not full */
        for (i = 0; i < LDV_RXBUFCOUNT; i ++)
        {
            if(RxBuffer[i].State == LdvRxBufferEmpty)
            {
                ASSERT_HRDY();
                break;
            }
        }
    }
}

/* 
 * Internal helper function to increment the various indices in a cyclic manner 
 */
static void CyclicIncrement(LdvIndexType index)
{
    switch (index)
    {
    case LdvIndexRxBufferReady:
        if (++DriverStatus.RxBufferReadyIndex >= LDV_RXBUFCOUNT)
            DriverStatus.RxBufferReadyIndex = 0;
        break;
    case LdvIndexRxBufferReceive:
        if (++DriverStatus.RxBufferReceiveIndex >= LDV_RXBUFCOUNT)
            DriverStatus.RxBufferReceiveIndex = 0;
        break;
    case LdvIndexTxBufferEmpty:
        if (++DriverStatus.TxBufferEmptyIndex >= LDV_TXBUFCOUNT)
            DriverStatus.TxBufferEmptyIndex = 0;
        break;
    case LdvIndexTxBufferTransmit:
        if (++DriverStatus.TxBufferTransmitIndex >= LDV_TXBUFCOUNT)
            DriverStatus.TxBufferTransmitIndex = 0;
        break;
    default:
        break;
    }
}

#ifdef PRINT_LINK_LAYER
static void PrintData(LonByte* pData, unsigned length, LonBool bTransmit)
{
    char sData[256];
    char sByte[4];
    unsigned i;
    if (bTransmit) 
        strcpy(sData, "Transmitted: ");
    else
        strcpy(sData, "Received   : ");
    for (i = 0; i < length; i ++)
    {
        sprintf(sByte, "%X ", pData[i]);
        strcat(sData, sByte);
    }
    printf("%s\n", sData);
}
#endif

/*
 * Function: LdvInit
 * Initialize the serial driver.
 * 
 * Remarks:
 * This function is called to initialize the serial driver.
 * Previously named ldv_init.
 */
void LdvInit(const char *usartName )
{
    LonUbits8 i;
    
    DriverStatus.DriverState            = LdvDriverSleep;
    DriverStatus.RxState                = LdvRxIdle;
    DriverStatus.RxBufferReceiveIndex   = 0;
    DriverStatus.RxBufferReadyIndex     = 0;
    DriverStatus.TxState                = LdvTxIdle;
    DriverStatus.TxBufferTransmitIndex  = 0;
    DriverStatus.TxBufferEmptyIndex     = 0;
    DriverStatus.pTxMsg                 = 0;
    DriverStatus.DrvWakeupTime          = LDV_DRVWAKEUPTIME;
    DriverStatus.RxTimeout              = 0;
    DriverStatus.KeepAliveTimeout       = LDV_KEEPALIVETIMEOUT;
    DriverStatus.PutMsgTimeout          = 0;

    /* Mark all receiver buffers as empty */
    for (i = 0; i < LDV_RXBUFCOUNT; i ++)
        RxBuffer[i].State = LdvRxBufferEmpty;

    /* Mark all transmit buffers as empty */
    for (i = 0; i < LDV_TXBUFCOUNT; i ++)
        TxBuffer[i].State = LdvTxBufferEmpty;
        
    // On ST Micro, UARTS and GPIOs (CTS/RTS/HRDY) have been initialized by the HAL.
    // Hardly any more work needs to be done here
    // but we do need to choose our serial port

    DEASSERT_RTS();
    DEASSERT_HRDY();

    /* Enable Global interrupts */
    EnableGlobalInterrupts();
}

/*
 * Function: LdvReset
 * Reset the serial driver.
 * 
 * Remarks:
 * This function is called to reset the serial driver.
 * The API resets the serial driver whenever it receives an uplink reset 
 * message from the Micro Server. A reset may also be required in cases when 
 * the driver detects some error in transmission or goes out of sync with the
 * Micro Server.
 * The driver should drop all pending receive and transmit transactions and go 
 * back to the initial state.
 */
void LdvReset(void) 
{
    DISABLE_TX_INT();
    DISABLE_RX_INT();
    DEASSERT_RTS();
    DEASSERT_HRDY();
    DriverStatus.DriverState = LdvDriverSleep;       /* Put the driver into sleep mode to begin with */
    DriverStatus.DrvWakeupTime = LDV_DRVWAKEUPTIME;  /* Actual reset will happen when the driver wakes up */
}

/*
 * Function: LdvGetMsg
 * Gets an incoming message from the serial driver.
 *
 * Parameters:
 * ppMsg - pointer to the transmit buffer pointer that contains the incoming 
 * message.
 *
 * Returns:
 * <LonApiError> - LonApiNoError if a message is available and was successfully 
 *                 returned, an appropriate error code, otherwise.
 *
 * Remarks:
 * This function gets an incoming message from the serial driver.
 * Note that the caller has a pointer into the driver memory upon a successful
 * return from the call. The caller must free the memory to the driver later 
 * by calling <LdvReleaseMsg>. 
 * Previously named ldv_get_msg.
 */
LonApiError LdvGetMsg(LonSmipMsg **ppMsg)
{
    LonApiError result = LonApiRxMsgNotAvailable;
    LonUbits8 i;
    
    for (i = 0; i < LDV_RXBUFCOUNT; i ++)
    {
        DisableGlobalInterrupts();
        if (RxBuffer[DriverStatus.RxBufferReadyIndex].State == LdvRxBufferReady)
        {
            RxBuffer[DriverStatus.RxBufferReadyIndex].State = LdvRxBufferProcessing;
            *ppMsg = (LonSmipMsg*) RxBuffer[DriverStatus.RxBufferReadyIndex].Data;
            EnableGlobalInterrupts();
            CyclicIncrement(LdvIndexRxBufferReady);
            result = LonApiNoError;
            break;
        }
        EnableGlobalInterrupts();
        CyclicIncrement(LdvIndexRxBufferReady);
    }
    
    return result;
}

/*
 * Function: LdvReleaseMsg
 * Releases a message buffer back to the serial driver.
 *
 * Parameters:
 * pMsg - pointer to the message buffer to be released.
 *
 * Remarks:
 * This function releases a message buffer back to the serial driver.
 * Note that the driver assumes that upon return, the memory pointed 
 * to by *pMsg* has been returned to the driver. Therefore, the caller 
 * must not use this memory anymore.
 * Previously named ldv_release_msg.
 */
void LdvReleaseMsg(const LonSmipMsg *pMsg)
{
    LonUbits8 i;
    const LonSmipMsg *pTMsg;
    
    for (i = 0; i < LDV_RXBUFCOUNT; i ++) 
    {
        DisableGlobalInterrupts();
        pTMsg = (const LonSmipMsg *)RxBuffer[i].Data;
        if (pTMsg == pMsg)
        {
            /* Found the message to be released in the buffer */
            RxBuffer[i].State = LdvRxBufferEmpty;
            EnableGlobalInterrupts();
            
            /* Resume ShortStack Micro Server if driver is in normal state */
            if (DriverStatus.DriverState == LdvDriverNormal)
                ASSERT_HRDY();
            break;
        }
        EnableGlobalInterrupts();
    }
}

/*
 * Function: LdvAllocateMsg
 * Allocates a transmit buffer from the serial driver.
 *
 * Parameters:
 * ppMsg - pointer to the transmit buffer pointer that will be returned.
 *
 * Returns:
 * <LonApiError> - LonApiNoError if the message was successfully allocated, 
 *                 an appropriate error code, otherwise.
 *
 * Remarks:
 * This function allocates a transmit buffer from the serial driver.
 * Note that the caller has a pointer into the driver memory upon a successful
 * return from the call. The caller must free the memory to the driver later 
 * by calling <LdvPutMsg>. 
 * Previously named ldv_allocate_msg.
 */
LonApiError LdvAllocateMsg(LonSmipMsg **ppMsg)
{
    LonApiError result = LonApiTxBufIsFull;
    LonUbits8 i;
    LonUbits8 *pMsg;
    
    for (i = 0; i < LDV_TXBUFCOUNT; i ++)
    {
        DisableGlobalInterrupts();
        if (TxBuffer[DriverStatus.TxBufferEmptyIndex].State == LdvTxBufferEmpty)
        {
            /* Found an empty message buffer */
            TxBuffer[DriverStatus.TxBufferEmptyIndex].State = LdvTxBufferFilling;
            pMsg = TxBuffer[DriverStatus.TxBufferEmptyIndex].Data;
            *ppMsg = (LonSmipMsg*) pMsg;
            EnableGlobalInterrupts();
            CyclicIncrement(LdvIndexTxBufferEmpty);
            result = LonApiNoError;
            break;
        }
        EnableGlobalInterrupts();
        CyclicIncrement(LdvIndexTxBufferEmpty);
    }
    
    if (result != LonApiNoError)
        nTxBufUnavailable ++;
    
    return result;
}

/*
 * Function: LdvPutMsg
 * Sends a message downlink.
 *
 * Parameters:
 * pMsg - pointer to the message that will be sent.
 *
 * Remarks:
 * This function sends a message downlink. The message must have been allocated
 * from the transmit buffer using <LdvAllocateMsg>. Note that the driver assumes
 * that upon return, the memory pointed to by *pMsg* has been returned to 
 * the driver. Therefore, the caller must not use this memory anymore.
 * Previously named ldv_put_msg.
 */
void LdvPutMsg(const LonSmipMsg* pMsg)
{
    LonUbits8 i;
    const LonSmipMsg *pTMsg;
   
    for (i = 0; i < LDV_TXBUFCOUNT; i ++)
    {
        DisableGlobalInterrupts();
        pTMsg = (const LonSmipMsg *)TxBuffer[i].Data;
        if (pTMsg == pMsg) 
        {
            /* Found the message to be transmitted */
            TxBuffer[i].State = LdvTxBufferReady;
            EnableGlobalInterrupts();
            break;
        }
        EnableGlobalInterrupts();
    }
}

/*
 * Function: LdvPutMsgBlocking
 * Sends a message downlink using a blocking call.
 *
 * Parameters:
 * pMsg - pointer to the message that will be sent.
 *
 * Returns:
 * <LonApiError> - LonApiNoError if the message was successfully sent, 
 *                 an appropriate error code, otherwise.
 *
 * Remarks:
 * This function sends a message downlink without allocating a transmit buffer 
 * from the serial driver. Note that this function blocks until the driver 
 * completes transmitting the message pointed to by *pMsg*. This function is 
 * typically used to send the initialization messages to the Micro Server.
 * Previously named ldv_put_msg_init.
 */
LonApiError LdvPutMsgBlocking(const LonSmipMsg* pMsg)
{
    LonApiError result = LonApiNoError;
    
    DriverStatus.PutMsgTimeout = LDV_PUTMSGTIMEOUT;
    while (TRUE)
    {    
        DisableGlobalInterrupts();
        if (DriverStatus.pTxMsg == 0)
        {
            /* There are no messages being transmitted, so transmit this message */
            DriverStatus.pTxMsg = (const LonByte *)pMsg;
            EnableGlobalInterrupts();
            break;
        }
        
        /* Check the timer */
        if (DriverStatus.PutMsgTimeout == 0)
        {
            /* The timer has expired. Declare the Micro Server as unresponsive */
            EnableGlobalInterrupts();
            result = LonApiMicroServerUnresponsive;
            break;
        }
        EnableGlobalInterrupts();
        
        /* If we got here, it means there are pending messages.
         * Transmit them first and then try to transmit pMsg. */
        LdvFlushMsgs();
        // todo 0 UpdateWatchDogTimer();
    }

    /* 
     * Our message is now in the driver.
     * Wait for the transmission to be finished.
     */
    while (DriverStatus.pTxMsg == (const LonByte*) pMsg)
    {
        LdvFlushMsgs();
        // todo 0 UpdateWatchDogTimer();
        
        /* Check the timer */
        DisableGlobalInterrupts();
        if (DriverStatus.PutMsgTimeout == 0)
        {
            /* The timer has expired. Declare the Micro Server as unresponsive */
            EnableGlobalInterrupts();
            result = LonApiMicroServerUnresponsive;
            break;
        }
        EnableGlobalInterrupts();
    }
    return result;
}

/*
 * Function: LdvFlushMsgs
 * Complete pending transmissions.
 *
 * Remarks:
 * This function must be called during the idle loop to complete pending 
 * transmissions.
 * Previously named ldv_flush_msgs.
 */
void LdvFlushMsgs(void)
{
    LonUbits8 i;

    if (CHECK_CTS_DEASSERTED()) /* If CTS is asserted, there is already a message being transmitted */
    {
        DisableGlobalInterrupts();
        if (DriverStatus.TxState == LdvTxIdle && DriverStatus.DriverState != LdvDriverSleep) 
        {
            /* The driver is awake and is ready to transmit */
            if (DriverStatus.pTxMsg != 0) 
            {
                /* There is already a message that needs to be transmitted.
                 * Assert the RTS line and then the interrupt routine will handle the transfer */
                ASSERT_RTS();
                EnableGlobalInterrupts();
            }
            else
            {
                /* The driver doesn't have any pending messages to transmit.
                 * Time to pull out any buffered messages */
                for (i = 0; i < LDV_TXBUFCOUNT; i ++) 
                {
                    DisableGlobalInterrupts();
                    if (TxBuffer[DriverStatus.TxBufferTransmitIndex].State == LdvTxBufferReady) 
                    {
                        /* Found a buffer that is ready for transmission */
                        DriverStatus.pTxMsg = TxBuffer[DriverStatus.TxBufferTransmitIndex].Data;
                        EnableGlobalInterrupts();
                        CyclicIncrement(LdvIndexTxBufferTransmit);
                        /* Assert the RTS line and then the interrupt routine will handle the transfer */
                        ASSERT_RTS();
                        break;
                    }
                    EnableGlobalInterrupts();
                    CyclicIncrement(LdvIndexTxBufferTransmit);
                }
            }
        }
        else
            EnableGlobalInterrupts();
    }
		osDelay(1);
}

/* 
 * Interrupt handler function for the CTS line
 */
void CtsInterruptHandler(void)
{
    /* Acknowledge the interrupt - not required for STMicro HAL done prior to this call */

    /* CTS line has changed since there is no other cause for this interrupt to occur */
    if (CHECK_CTS_ASSERTED())
    {
        /* Micro Server is ready to receive data */
        ENABLE_TX_INT();
        DEASSERT_RTS();     /* No need to keep RTS asserted any more */
    }
    else /* CTS line has been deasserted */
    if (DriverStatus.TxState == LdvTxHandShake)
    {
        /* The header has been fully transmitted and info/payload needs to be sent */
        ASSERT_RTS();            /* Request to send info or payload */
        if (DriverStatus.TxNextState == LdvTxInfo_1)
            DriverStatus.TxState = LdvTxInfo_1;
        else if (DriverStatus.TxNextState == LdvTxPayload)
            DriverStatus.TxState = LdvTxPayload;
    }
    /* Else end of transmission of a packet, nothing needs to be done */
}

/* 
 * Interrupt handler function for the receiver and transmitter interrupts
 */
void RxInterruptHandler(LonByte data)
{
				int i = 0;

        LonByte rxChar;
        rxChar = data;  /* Read data */

        if (DriverStatus.DriverState == LdvDriverNormal)
        {
            /* Driver is in good shape, handle the data */
            switch (DriverStatus.RxState)
            {
            case LdvRxIdle:
                /* Length byte has arrived */
                /* Initially assume no receive buffer is available */
                DriverStatus.RxState = LdvRxIgnore;

                /* rxChar contains the length byte, total payload size is length byte + 1 byte for the command */
                DriverStatus.RxPayloadLen = rxChar + 1;
                DriverStatus.RxTimeout = LDV_RXTIMEOUT;

                if (rxChar < (LDV_RXBUFSIZE - sizeof(LonSmipHdr)))
                {
                    /* Find an empty buffer to store this data */
                    for (i = 0; i < LDV_RXBUFCOUNT; i ++)
                    {
                        LonByte rcvIndex = DriverStatus.RxBufferReceiveIndex;
                        if (RxBuffer[rcvIndex].State == LdvRxBufferEmpty)
                        {
                            /* Found an empty buffer */
                            DriverStatus.RxNextFree = 0;                        /* Initialize the buffer receive counter */
                            DriverStatus.RxState = LdvRxPayload;                /* Change the driver receive state */
                            RxBuffer[rcvIndex].State = LdvRxBufferReceiving;    /* Change the buffer state */
                            RxBuffer[rcvIndex].Data[DriverStatus.RxNextFree ++] = rxChar;  /* Store the data in the buffer */

                            break;
                        }
                        CyclicIncrement(LdvIndexRxBufferReceive);
                    }
                }
                break;

            case LdvRxPayload:
            {
                /* Payload is arriving, keep pushing data in the receive buffer */
                LonByte rcvIndex = DriverStatus.RxBufferReceiveIndex;
                RxBuffer[rcvIndex].Data[DriverStatus.RxNextFree ++] = rxChar;
                /* Check if all the bytes have been received */
                if (-- DriverStatus.RxPayloadLen == 0)
                {
                    /* All bytes have been received */
                    #ifdef PRINT_LINK_LAYER
                    PrintData((LonByte*) &RxBuffer[rcvIndex].Data[0], DriverStatus.RxNextFree, 0);
                    #endif
                    DriverStatus.RxState = LdvRxIdle;
                    RxBuffer[rcvIndex].State = LdvRxBufferReady;
                    DriverStatus.RxTimeout = 0;
                    CyclicIncrement(LdvIndexRxBufferReceive);

                    /* Check if there is still an empty receive buffer */
                    for (i = 0; i < LDV_RXBUFCOUNT; i ++)
                        if (RxBuffer[i].State == LdvRxBufferEmpty)
                            /* There is an empty receive buffer, we are done */
                            break;
                    /* If there is no empty receive buffer, quench the neuron */
                    if (i == LDV_RXBUFCOUNT) {
                        DEASSERT_HRDY();
                    }
                }
                else
                {
                    DriverStatus.RxTimeout = LDV_RXTIMEOUT;
                }
                break;
            }

            case LdvRxIgnore:
                /* There was no receive buffer available to store this message */
                /* Keep ignoring till the full message has arrived */
                if (-- DriverStatus.RxPayloadLen == 0)
                {
                    DriverStatus.RxState = LdvRxIdle;
                    DriverStatus.RxTimeout = 0;
                }
                else
                {
                    DriverStatus.RxTimeout = LDV_RXTIMEOUT;
                }
                break;

            default:
                break;
            }
            /* Restart the keep-alive timer */
            DriverStatus.KeepAliveTimeout = LDV_KEEPALIVETIMEOUT;
        }
}



void TxInterruptHandler(void)
{

			static uint8_t pdata;
			int i = 0;

        /* If data needs to be transmitted, make sure that the CTS line is still asserted */
      if ( (DriverStatus.TxState != LdvTxDone)  &&  CHECK_CTS_DEASSERTED() )
        {
            /* Something unusual happened (such as Neuron reset) */
            /* Reset the Micro Server and start over. Once the Micro Server resets,
               the uplink reset message will come in and it will reset this driver also  */
            ResetMicroServer();
        }
        else
        {
            const LonByte* pTM = DriverStatus.pTxMsg;
            switch (DriverStatus.TxState)
            {
            case LdvTxIdle:
                /* Transmit length */
                if (pTM != 0)
                {
                    for (i = 0; i < LDV_TXBUFCOUNT; i ++)
                    {
                        /* See if this message belongs to any buffer.
                         * If found and the buffer is ready to transmit, change its state */
                        if ((TxBuffer[i].Data == pTM) && (TxBuffer[i].State == LdvTxBufferReady))
                        {
                            TxBuffer[i].State = LdvTxBufferTransmitting;
                            break;
                        }
                    }
                    DriverStatus.TxNextChar = 0;
                    DriverStatus.TxPayloadLen = pTM[0];   /* The first byte in the message is the payload length */
                    DriverStatus.TxState = LdvTxCmd;
                    pdata = pTM[DriverStatus.TxNextChar++];
                    HAL_UART_Transmit_IT(&FT_UART, &pdata, 1);
                }
                break;

            case LdvTxCmd:
                /* Transmit command */
                pdata = pTM[DriverStatus.TxNextChar++];
                HAL_UART_Transmit_IT(&FT_UART, &pdata, 1);
                /* Two byte header has been sent */
                /* First, check to see if info byte needs to be sent */
                if (pTM[DriverStatus.TxNextChar-1] == (LonNiNv | LON_NV_ESCAPE_SEQUENCE))
                {
                    DriverStatus.TxState = LdvTxHandShake; /* As soon as the CTS is deasserted, we will assert RTS to send the info */
                    DriverStatus.TxNextState = LdvTxInfo_1;
                    DISABLE_TX_INT();                   /* Disable transmit interrupt */
                }
                /* Check if there is payload to be sent */
                else if (DriverStatus.TxPayloadLen != 0)
                {
                    DriverStatus.TxState = LdvTxHandShake; /* As soon as the CTS is deasserted, we will assert RTS to send the payload */
                    DriverStatus.TxNextState = LdvTxPayload;
                    DISABLE_TX_INT();                   /* Disable transmit interrupt */
                }
                else
                {
                    DriverStatus.TxState = LdvTxDone;
                }
                break;

            case LdvTxInfo_1:
                /* Transmit the first info byte */
                pdata = ((LonSicb*) ((LonSmipMsg*) pTM)->Payload)->NvMessage.Index ;
                HAL_UART_Transmit_IT(&FT_UART, &pdata, 1);
                DriverStatus.TxState = LdvTxInfo_2;
                break;

            case LdvTxInfo_2:
                /* Transmit the second info byte */
                /* For now, this byte is 0x00 */
                pdata = 0x00;
                HAL_UART_Transmit_IT(&FT_UART, &pdata, 1);
                /* Info bytes have been sent */
                /* Check if there is payload to be sent */
                if (DriverStatus.TxPayloadLen != 0)
                {
                    DriverStatus.TxState = LdvTxHandShake; /* As soon as the CTS is deasserted, we will assert RTS to send the payload */
                    DriverStatus.TxNextState = LdvTxPayload;
                    DISABLE_TX_INT();                   /* Disable transmit interrupt */
                }
                else
                {
                    DriverStatus.TxState = LdvTxDone;
                }
                break;

            case LdvTxPayload:
                /* Keep transmitting payload */
                pdata = pTM[DriverStatus.TxNextChar++] ;
                HAL_UART_Transmit_IT(&FT_UART, &pdata, 1);
                if (-- DriverStatus.TxPayloadLen == 0)
                {
                    DriverStatus.TxState = LdvTxDone;
                }
                break;

            case LdvTxDone:
                #ifdef PRINT_LINK_LAYER
                PrintData((LonByte*) pTM, DriverStatus.TxNextChar, 1);
                #endif
                for (i = 0; i < LDV_TXBUFCOUNT; i ++)
                {
                    /* Find the buffer which was being transmitted if any, and if found, change its state to empty */
                    if (TxBuffer[i].State == LdvTxBufferTransmitting)
                    {
                        TxBuffer[i].State = LdvTxBufferEmpty;
                        break;
                    }
                }
                DriverStatus.pTxMsg = 0;
                DISABLE_TX_INT();        /* We are done transmitting, disable transmit interrupt */
                DriverStatus.TxState = LdvTxIdle;
                break;

            default:
                break;
            }
            /* Restart the keep-alive timer */
            DriverStatus.KeepAliveTimeout = LDV_KEEPALIVETIMEOUT;
        }
}

/*
 * Interrupt handler function for the periodic interval timer
 */
void PeriodicIntervalTimerHandler1ms(void)
{
    LonUbits8 i;
    LonUbits32 status;
    /* Acknowledge the interrupt */

    /* First, handle the utility timers */
    for (int i = 0; i < 4; i ++)
    {
        if (nUtilityUpTimer[i] < MAX_UNSIGNED)
            nUtilityUpTimer[i] ++;
        if (nUtilityDownTimer[i])
            nUtilityDownTimer[i] --;
    }

    /* Check the receiver timer */
    if (DriverStatus.RxTimeout != 0)
    {
        /* It means this timer is set */
        /* Decrement one unit and see if it is supposed to expire */
        if (-- DriverStatus.RxTimeout == 0)
        {
            /* Receive timer has expired! */
            nRxTimeout ++;
            /* The driver and the Micro Server may have become out of sync */
            /* Reset the Micro Server and start over. Once the Micro Server resets,
               the uplink reset message will come in and it will reset this driver also  */
            ResetMicroServer();
        }
    }

    /* Check the driver sleep timer */
    if (DriverStatus.DrvWakeupTime != 0)
    {
        /* It means this timer is set */
        /* Decrement one unit and see if it is supposed to expire */
        if (--DriverStatus.DrvWakeupTime == 0)
        {
            /* The timer has expired. Wake up the driver */
            DriverStatus.DriverState    = LdvDriverNormal;
            DriverStatus.TxState        = LdvTxIdle;
            DriverStatus.RxState        = LdvRxIdle;

            /* Abort ongoing receive transactions if any */
            for (i = 0; i < LDV_RXBUFCOUNT; i ++)
                if (RxBuffer[i].State == LdvRxBufferReceiving)
                    RxBuffer[i].State = LdvRxBufferEmpty;
            /* Restart ongoing transmit transactions if any */
            for (i = 0; i < LDV_TXBUFCOUNT; i ++)
                if (TxBuffer[i].State == LdvTxBufferTransmitting)
                    TxBuffer[i].State = LdvTxBufferReady;

            /* Clear interrupts if any, by reading the status register */

            ENABLE_RX_INT();        /* Enable receive interrupt */
            ASSERT_HRDY();          /* Resume Neuron */
        }
    }

    /* Check the keep-alive timer */
    if (DriverStatus.KeepAliveTimeout != 0)
    {
        if (--DriverStatus.KeepAliveTimeout == 0)
        {
            /* Keep-alive timer has expired. Deassert and then assert the HRDY */
            DEASSERT_HRDY();
            SleepMs(1);
            ASSERT_HRDY();
            /* Restart the timer */
            DriverStatus.KeepAliveTimeout = LDV_KEEPALIVETIMEOUT;
        }
    }
    
    /* Check the put message blocking timer */
    if (DriverStatus.PutMsgTimeout != 0)
    {
        --DriverStatus.PutMsgTimeout;
    }
}
