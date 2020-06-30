/****************************************************************************************
*
*   Copyright (C) 2020 ConnectEx, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   As a special exception, if other files instantiate templates or
*   use macros or inline functions from this file, or you compile
*   this file and link it with other works to produce a work based
*   on this file, this file does not by itself cause the resulting
*   work to be covered by the GNU General Public License. However
*   the source code for this file must still be made available in
*   accordance with section (3) of the GNU General Public License.
*
*   This exception does not invalidate any other reasons why a work
*   based on this file might be covered by the GNU General Public
*   License.
*
*   For more information: info@connect-ex.com
*
*   For access to source code :
*
*       info@connect-ex.com
*           or
*       github.com/ConnectEx/BACnet-Dev-Kit
*
****************************************************************************************/

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
