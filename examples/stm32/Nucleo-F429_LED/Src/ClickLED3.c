/*
 * ClickLED3.c
 *
 *  Created on: Sep 29, 2019
 *      Author: Roberto
 */

#include <main.h>
#include <stdio.h>
#include <string.h>

#include "ClickLED3.h"

// #define for UART output
#undef CLICKLED3_SERIAL_DEBUG

extern I2C_HandleTypeDef hi2c1;

#ifdef CLICKLED3_SERIAL_DEBUG
extern UART_HandleTypeDef huart3;
#endif

HAL_StatusTypeDef ClickLED3_Reset() {
	HAL_StatusTypeDef ret;

	uint8_t buf[12];

	buf[0] = ClickLED3_SHUTDWN;
	ret = HAL_I2C_Master_Transmit(&hi2c1, ClickLED3_ADDR, buf, 1, HAL_MAX_DELAY);

#ifdef CLICKLED3_SERIAL_DEBUG
	if (ret != HAL_OK){
		strcpy((char*) buf, "Error\r\n");
	}
	else{
		strcpy((char*) buf, "Sent\r\n");
	}
    HAL_UART_Transmit(&huart3, buf, strlen((char*)buf), HAL_MAX_DELAY);
#endif

	return ret;
}


HAL_StatusTypeDef ClickLED3_SetIntensity(uint8_t intensity) {
	HAL_StatusTypeDef ret;

	uint8_t buf[12];

	buf[0] = ClickLED3_LEDCURR | intensity >> 3;  // Max intensity (max 5 bits)
	ret = HAL_I2C_Master_Transmit(&hi2c1, ClickLED3_ADDR, buf, 1, HAL_MAX_DELAY);

#ifdef CLICKLED3_SERIAL_DEBUG
	if (ret != HAL_OK){
		strcpy((char*) buf, "Error\r\n");
	}
	else{
		strcpy((char*) buf, "Sent\r\n");
	}

    HAL_UART_Transmit(&huart3, buf, strlen((char*)buf), HAL_MAX_DELAY);
#endif

    return ret;
}

HAL_StatusTypeDef ClickLED3_SetRGB(uint8_t red, uint8_t green, uint8_t blue) {
	HAL_StatusTypeDef ret;

	  uint8_t buf[12];

	  // NCP5623B-D only accepts one command byte on each transmission

	  // Individual LEDs

	  buf[0] = ClickLED3_PWMLED1 | red >> 3; // Led 1 Red intensity (3 LSb ignored)
	  ret = HAL_I2C_Master_Transmit(&hi2c1, ClickLED3_ADDR, buf, 1, HAL_MAX_DELAY);

	  buf[0] = ClickLED3_PWMLED2 | green >> 3; // Led 2 Green intensity (3 LSb ignored)
	  ret = HAL_I2C_Master_Transmit(&hi2c1, ClickLED3_ADDR, buf, 1, HAL_MAX_DELAY);

	  buf[0] = ClickLED3_PWMLED3 | blue >> 3; // Led 3 Blue intensity (3 LSb ignored)
	  ret = HAL_I2C_Master_Transmit(&hi2c1, ClickLED3_ADDR, buf, 1, HAL_MAX_DELAY);

#ifdef CLICKLED3_SERIAL_DEBUG
	  if (ret != HAL_OK){
		  strcpy((char*) buf, "Error\r\n");
	  }
	  else{
		  strcpy((char*) buf, "Sent\r\n");
	  }
	  HAL_UART_Transmit(&huart3, buf, strlen((char*)buf), HAL_MAX_DELAY);
#endif

	  return ret;
}
