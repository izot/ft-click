#ifndef _FTMQ_H
#define _FTMQ_H

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F103xB
	#include "stm32f1xx_hal.h"
	#define UART_FT huart3
	#define UART_FT_INSTANCE ((USART_TypeDef *) USART3_BASE)
#endif
#ifdef STM32F429xx
	#include "stm32f4xx_hal.h"
	#define UART_FT huart6
	#define UART_FT_INSTANCE ((USART_TypeDef *) USART6_BASE)
#endif
#ifndef UART_FT
	#error "Not supported platform. Only STM32F4 and STM32F1 are supported at this moment."
#endif

#define MAX_SUSCTIPTIONS	10

struct protoFtmq
{
    uint8_t address;
    union
    {
        float   value;
        uint8_t valuebytes[4];
    };
};

typedef void (*callbackFunc) (float arg1);
void (*callbackFuncArray[MAX_SUSCTIPTIONS]) (float value);

void protoFtmqSetup();
void protoFtmqCheck();
void protoFtmqsend(uint8_t address, float value);
void protoFtmqSubscribe(uint8_t address, callbackFunc func);


#endif /* _FTMQ_H */
