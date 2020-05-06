/*
 * debug.h
 *
 *  Created on: 2 oct. 2019
 *      Author: Roberto
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <string.h>

#ifndef SERIAL_DEBUG_UART
#ifdef STM32F103xB
#include "stm32f1xx_hal.h"
#define SERIAL_DEBUG_UART huart2
#endif
#ifdef STM32F429xx
#include "stm32f4xx_hal.h"
#define SERIAL_DEBUG_UART huart3
#endif

#ifndef SERIAL_DEBUG_UART
#error "Not supported platform. Only STM32F4 and STM32F1 are supported at this moment."
#endif
#endif

extern UART_HandleTypeDef SERIAL_DEBUG_UART;

#ifndef SERIAL_DEBUG
#define SERIAL_DEBUG(s) HAL_UART_Transmit(&SERIAL_DEBUG_UART, (uint8_t *)s, strlen(s), HAL_MAX_DELAY);
#endif
char debug_buf[256];

#define SERIAL_DEBUG_SPRINTF_1(s,a) sprintf(debug_buf, s, a); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_2(s,a1,a2) sprintf(debug_buf, s, a1, a2); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_3(s,a1,a2,a3) sprintf(debug_buf, s, a1, a2, a3); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_4(s,a1,a2,a3,a4) sprintf(debug_buf, s, a1, a2, a3, a4); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_5(s,a1,a2,a3,a4,a5) sprintf(debug_buf, s, a1, a2, a3, a4, a5); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_6(s,a1,a2,a3,a4,a5,a6) sprintf(debug_buf, s, a1, a2, a3, a4, a5, a6); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_7(s,a1,a2,a3,a4,a5,a6,a7) sprintf(debug_buf, s, a1, a2, a3, a4, a5, a6, a7); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_8(s,a1,a2,a3,a4,a5,a6,a7,a8) sprintf(debug_buf, s, a1, a2, a3, a4, a5, a6, a7, a8); SERIAL_DEBUG(debug_buf);
#define SERIAL_DEBUG_SPRINTF_9(s,a1,a2,a3,a4,a5,a6,a7,a8,a9) sprintf(debug_buf, s, a1, a2, a3, a4, a5, a6, a7, a8, a9); SERIAL_DEBUG(debug_buf);

#endif /* DEBUG_H */
