#ifndef COMMON_H
#define COMMON_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "ccp_stm32.h"

extern uint8_t uartcRxchar;

extern UART_HandleTypeDef huart6;
#define CLICK_UART huart6


#define DEBUG_ON

#ifdef DEBUG_ON
  char debug_buf[256];
  #define DEBUGPRINT(...) sprintf(debug_buf, __VA_ARGS__); HAL_UART_Transmit(&VCOM_UART, (uint8_t *)debug_buf, strlen(debug_buf), HAL_MAX_DELAY);
#else
  #define DEBUGPRINT(...)
#endif

#endif
