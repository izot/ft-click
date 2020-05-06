/*
 * Filename: LdvSci.h
 *
 * Description:  This file contains the enumerations and types required 
 * for the ARM7 ShortStack SCI driver to interface with a ShortStack Micro Server.
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

#ifndef LDV_SCI_H
#define LDV_SCI_H

// #include "Processor.h"
#include "ShortStackDev.h"
#include "ShortStackApi.h"

#if defined(STM32F103xB)
	#include "stm32f1xx_hal.h"
#elif defined(STM32F429xx)
	#include "stm32f4xx_hal.h"
#elif defined(STM32G071xx)
  #include "stm32g0xx_hal.h"
  #include "stm32g071xx.h"
#else
	#error "Not supported platform."
#endif

#include "main.h"

/*
 * Pull in platform specific pragmas, definitions etc. 
 * For example, packing directives to align objects on byte boundary.
 */
#ifdef  INCLUDE_LON_BEGIN_END
#   include "LonBegin.h"    
#endif  /* INCLUDE_LON_BEGIN_END */

/******************************************************************************************
 * Define the driver's receive/transmit buffer size and count.
 *
 * LDV_RXBUFSIZE must be no less than the maximum size of possible uplink messages.
 * Choose the value according to the following formular:
 *
 *    LDV_RXBUFSIZE = minimum(input application buffer size on shortStack Micro Server,
 *                            message_size + 5 bytes of system overhead)
 *
 * If explicit addressing is used, add an additional 11 bytes of system overhead.
 *
 * For explicit messages, message_size equals 1 bytes for the message code plus the
 * number of data. For the network variables, message_size equals 2 bytes plus the 
 * number of bytes in the network variable.
 *
 * FYI: If the host application doesn't use explicit message and explicit addressing
 *        is off on the ShortStack Micro Server, configure LDV_RXBUFSIZE:
 *        
 *            LDV_RXBUFSIZE = (7+maxixum NV size in bytes)
 *        
 * LDV_TXBUFSIZE must be no less than the maximum size of possible downlink messages
 * and no more than output application buffer size defined on the ShortStack Micro Server.
 * Use the same formular above to determine the appropriate size.
 *
 * FYI: If the host application is to support LonTalk FTP, match driver's receive/transmit
 *        buffer size with input/output application buffer size defined on the ShortStack
 *        Micro Server respectively. (To support FTP, the host application will need to exploit
 *        explicit addressed explicit messages.)
 *******************************************************************************************/
#define LDV_RXBUFSIZE           LON_APP_INPUT_BUFSIZE
#define LDV_TXBUFSIZE           LON_APP_OUTPUT_BUFSIZE
#define LDV_RXBUFCOUNT          5
#define LDV_TXBUFCOUNT          5

/* The following literal selects the bit rate of the SCI communication        
 * interface on the host processor. The following table gives the bit rate    
 * selection for the ARM7 processor running on the Pyxos EV-Pilot board       
 *                                                                            
 * SS_BAUD_RATE           SCI(bps)                                            
 *        0                76 800 (default)                                   
 *        1                38 400
 *        2				   19 200
 *        3				    9 600
 *                                                                            
 * Note : The value for this literal must match the actual bit rate defined   
 *         for the ShortStack Micro Server.                                   
 */
#define SS_BAUD_RATE            1

/*
 * Specify timeout value for the receiver, and the wakeup time value 
 * for the driver. MUST adjust these values according to actual SCI baud rate.
 * Typically, set LDV_RXTIMEOUT equal to 160 times of bit time (16 bytes including 
 * start and stop bits), and LDV_DRVWAKEUPTIME a whole message time.
 * Note that these timeouts are in milliseconds.
 */
#if (SS_BAUD_RATE == 0)
    #define LDV_RXTIMEOUT       2
#elif (SS_BAUD_RATE == 1)
    #define LDV_RXTIMEOUT       3	// just a guess...
#elif (SS_BAUD_RATE == 2)
    #define LDV_RXTIMEOUT       3	// and another guess
#elif (SS_BAUD_RATE == 3)
    #define LDV_RXTIMEOUT       4
#else
#error
#endif
#define LDV_DRVWAKEUPTIME       255 

/*
 * Specify the keepalive timeout value for the serial link.
 * If there is no activity on the serial lines for an extended period of time, 
 * the driver will nudge the Micro Server to tell it that the host is still active. 
 * This isn't required by the Micro Server, but may be required by the underlying 
 * hardware (e.g., RS-232 chip)
 * Note that this timeout is in milliseconds.
 */
#define LDV_KEEPALIVETIMEOUT    20000

/*
 * Specify the put message blocking timeout value for the serial link.
 * If the driver is unable to send the message downlink for this timeout value
 * it assumes that the Micro Server isn't in a position to receive the message,
 * possibly because it is stuck or not present at all. 
 * Note that this timeout is in milliseconds.
 */
#define LDV_PUTMSGTIMEOUT       60000

/*
 * Define the ARM7 pins used for SCI communication.
 */
//#define SBR0                    AT91C_PIO_PA0
//#define SBR1                    AT91C_PIO_PA1
//#define HRDY                    AT91C_PIO_PA2
//#define RXD                     AT91C_PA5_RXD0
//#define TXD                     AT91C_PA6_TXD0
//#define RTS                     AT91C_PA7_RTS0
//#define CTS                     AT91C_PA8_CTS0
//#define NEURON_RESET            AT91C_PIO_PA23
//
//#define COM0                    (AT91C_BASE_US0)    /* Using USART0 channel of the ARM7 */
//#define CSR                     (COM0->US_CSR)      /* Status register */
//#define IMR                     (COM0->US_IMR)      /* Interrupt mask register */

#define CHECK_CTS_DEASSERTED() 	(HAL_GPIO_ReadPin(SS_I_CTS_GPIO_Port, SS_I_CTS_Pin))
#define CHECK_CTS_ASSERTED()    (!CHECK_CTS_DEASSERTED())

#define DEASSERT_RTS()          (HAL_GPIO_WritePin(SS_O_RTS_GPIO_Port, SS_O_RTS_Pin, GPIO_PIN_SET))
#define ASSERT_RTS()          	(HAL_GPIO_WritePin(SS_O_RTS_GPIO_Port, SS_O_RTS_Pin, GPIO_PIN_RESET))
#define CHECK_RTS_DEASSERTED()  (HAL_GPIO_ReadPin(SS_O_RTS_GPIO_Port, SS_O_RTS_Pin))

#ifdef FT_HRDY_Pin
	#define DEASSERT_HRDY()         (HAL_GPIO_WritePin(FT_HRDY_GPIO_Port, FT_HRDY_Pin, GPIO_PIN_SET))
	#define ASSERT_HRDY()           (HAL_GPIO_WritePin(FT_HRDY_GPIO_Port, FT_HRDY_Pin, GPIO_PIN_RESET))
#else  // HRDY line can be attached to GND if the host server is always ready
	#warning No FT_HRDY line defined
	#define DEASSERT_HRDY()
	#define ASSERT_HRDY()
#endif

#ifdef FT_RESET_Pin
	#define DEASSERT_RESET()         (HAL_GPIO_WritePin(FT_RESET_GPIO_Port, FT_RESET_Pin, GPIO_PIN_SET))
	#define ASSERT_RESET()	         (HAL_GPIO_WritePin(FT_RESET_GPIO_Port, FT_RESET_Pin, GPIO_PIN_RESET))
#else
	#warning No FT_RESET line defined
	#define DEASSERT_RESET()
	#define ASSERT_RESET()
#endif

//#define ENABLE_RX_INT()         (AT91F_US_EnableIt(COM0, AT91C_US_RXRDY | AT91C_US_OVRE | AT91C_US_FRAME | AT91C_US_PARE))

#ifdef STM32G071xx
	extern LonByte rxNeuronData;
	extern LonBool neuron_rxint;
	#define DISABLE_RX_INT()        (neuron_rxint = FALSE)/* (huart1.Instance->CR1 &= ~(USART_CR1_RXNEIE_RXFNEIE_Msk))*/
	#define ENABLE_RX_INT()			(neuron_rxint = TRUE); HAL_UART_Receive_IT(&FT_UART, &rxNeuronData, 1)/*(huart1.Instance->CR1 |= (USART_CR1_RXNEIE_RXFNEIE_Msk))*/
	#define NEURON_RXINT_ENABLED()    (neuron_rxint)
#else
	#define DISABLE_RX_INT()        (huart1.Instance->CR1 &= ~(USART_CR1_RXNEIE_Msk))
	#define ENABLE_RX_INT()			(huart1.Instance->CR1 |= (USART_CR1_RXNEIE_Msk))
#endif


#ifdef STM32G071xx
	extern LonBool neuron_txint;
	#define ENABLE_TX_INT()       (neuron_txint = TRUE);         TxInterruptHandler()/*  (huart1.Instance->CR1 |= (USART_CR1_TXEIE_TXFNFIE_Msk))*/
	#define DISABLE_TX_INT()       (neuron_txint = FALSE)/* (huart1.Instance->CR1 &= ~(USART_CR1_TXEIE_TXFNFIE_Msk))*/
    #define NEURON_TXINT_ENABLED()    (neuron_txint)
#else
	//#define ENABLE_TX_INT()         (huart2.CR1 |= (USART_CR1_TXEIE_Msk))
	//#define DISABLE_TX_INT()        (huart2.CR1 &= ~(USART_CR1_TXEIE_Msk))
	#define ENABLE_TX_INT()         (huart1.Instance->CR1 |= (USART_CR1_TXEIE_Msk))
	#define DISABLE_TX_INT()        (huart1.Instance->CR1 &= ~(USART_CR1_TXEIE_Msk))
#endif

void TxInterruptHandler(void);
void RxInterruptHandler(LonByte data);

#define SleepMs(a)	osDelay(a)

/* SCI transmitter state    */
typedef LON_ENUM_BEGIN(LdvTxStates)
{
    LdvTxIdle       = 0,    /* Transmitter is idle */
    LdvTxCmd,               /* The command byte needs to be transmitted */
    LdvTxHandShake,         /* Need to perform another handshake */
    LdvTxInfo_1,            /* The first info byte needs to be transmitted */
    LdvTxInfo_2,            /* The second info byte needs to be transmitted */
    LdvTxPayload,           /* Transmitter is in the middle of sending the message payload */
    LdvTxDone               /* Have completed sending the entire message */
} LON_ENUM_END(LdvTxStates);

/* SCI receiver state    */
typedef LON_ENUM_BEGIN(LdvRxStates)
{
    LdvRxIdle       = 0,    /* Receiver is idle */
    LdvRxPayload,           /* The length byte has been received, waiting for the rest of the message */
    LdvRxIgnore             /* Ignore the rest of the message */
} LON_ENUM_END(LdvRxStates);

/* Serial driver state    */
typedef LON_ENUM_BEGIN(LdvDriverState)
{
    LdvDriverSleep   = 0,        /* The driver is disabled */
    LdvDriverNormal  = 1         /* The driver is ready to receive and transmit messages */
} LON_ENUM_END(LdvDriverState);

/* Serial driver status    */
typedef LON_STRUCT_BEGIN(LdvDriverStatus)
{
    LdvDriverState      DriverState;
    LdvRxStates         RxState;
    LonByte             RxBufferReadyIndex;     /* Receive buffer index which contains an incoming message */
    LonByte             RxBufferReceiveIndex;   /* Receive buffer index which is in the process of receiving 
                                                 * an incoming message */
    LonByte             RxNextFree;             /* Position into the receive buffer where the next incoming
                                                 * byte will be stored */
    LonByte             RxPayloadLen;           /* Size of the incoming message payload */
    LdvTxStates         TxState;
    LdvTxStates         TxNextState;
    LonByte             TxBufferEmptyIndex;     /* Transmit buffer index which is empty */
    LonByte             TxBufferTransmitIndex;  /* Transmit buffer index which is in the process of
                                                 * transmitting an outgoing message */
    LonByte             TxNextChar;             /* Position into the transmit buffer where the next byte to be
                                                 * transmitted is stored */
    LonByte             TxPayloadLen;           /* Size of the outgoing message payload */
    const LonByte*      pTxMsg;                 /* Pointer to the message being transmitted or to be transmitted */
    LonByte             RxTimeout;              /* Receiver timer. If the receiver timer expires while waiting
                                                 * for an incoming message, abort the ongoing receive transaction
                                                 * and reset the SCI */
    LonByte             DrvWakeupTime;          /* A period of time the driver waits before bringing the SCI back
                                                 * to normal after resetting the SCI */
    LonUbits32          KeepAliveTimeout;       /* Keep-alive timer. If there is no activity on the serial lines for
                                                 * an extended period of time, nudge the Micro Server to tell it that the 
                                                 * host is still active. This isn't required by the Micro Server, but
                                                 * may be required by the underlying hardware (e.g., RS-232 chip) */
    LonUbits32          PutMsgTimeout;          /* Put message blocking timeout. */
} LON_STRUCT_END(LdvDriverStatus);

/* Receive buffer state    */
typedef LON_ENUM_BEGIN(LdvRxBufferState)
{
    LdvRxBufferEmpty,        /* Buffer is empty */
    LdvRxBufferReceiving,    /* Buffer is being used by driver for receiving */
    LdvRxBufferReady,        /* Buffer is ready for processing */
    LdvRxBufferProcessing    /* Buffer is being processed by the API */
} LON_ENUM_END(LdvRxBufferState);

/* Definition of receive buffer    */
typedef LON_STRUCT_BEGIN(LdvSysRxBuffer)
{
    LdvRxBufferState    State;
    LonByte             Data[LDV_RXBUFSIZE];
} LON_STRUCT_END(LdvSysRxBuffer);

/* Transmit buffer state    */
typedef LON_ENUM_BEGIN(LdvTxBufferState)
{
    LdvTxBufferEmpty,        /* Buffer is empty */
    LdvTxBufferFilling,      /* Buffer is being filled by the API */
    LdvTxBufferReady,        /* Buffer is ready for transmitting */
    LdvTxBufferTransmitting  /* Buffer is being transmitted by the driver */
} LON_ENUM_END(LdvTxBufferState);

/* Definition of transmit buffer    */
typedef LON_STRUCT_BEGIN(LdvSysTxBuffer)
{
    LdvTxBufferState    State;
    LonByte             Data[LDV_TXBUFSIZE];
} LON_STRUCT_END(LdvSysTxBuffer);

/* Types of index used for incrementing. See function CyclicIncrement */
typedef LON_ENUM_BEGIN(LdvIndexType)
{
    LdvIndexRxBufferReady,
    LdvIndexRxBufferReceive,
    LdvIndexTxBufferEmpty,
    LdvIndexTxBufferTransmit
} LON_ENUM_END(LdvIndexType);

/*
 * Restore packing directives.
 */
#ifdef  INCLUDE_LON_BEGIN_END
#   include "LonEnd.h"    
#endif  /* INCLUDE_LON_BEGIN_END */

#endif /* LDV_SCI_H */
