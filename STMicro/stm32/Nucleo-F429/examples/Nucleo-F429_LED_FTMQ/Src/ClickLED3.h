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
 * ClickLED3.h
 * Click LED 3
 *  Created on: Sep 29, 2019
 *      Author: Roberto
 */

#ifndef CLICKLED3_H
#define CLICKLED3_H

#include <main.h>

#define ClickLED3_ADDR 0x38 << 1

#define ClickLED3_SHUTDWN 0x00
#define ClickLED3_LEDCURR 0x20
#define ClickLED3_PWMLED1 0x40
#define ClickLED3_PWMLED2 0x60
#define ClickLED3_PWMLED3 0x80
#define ClickLED3_DIMUPSET 0xA0
#define ClickLED3_DIMDWNSET 0xC0
#define ClickLED3_DIMTIME 0xE0


HAL_StatusTypeDef ClickLED3_Reset();
HAL_StatusTypeDef ClickLED3_SetIntensity(uint8_t intensity);
HAL_StatusTypeDef ClickLED3_SetRGB(uint8_t red, uint8_t green, uint8_t blue);

#endif /* CLICKLED3_H */
