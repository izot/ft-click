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
