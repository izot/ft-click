/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#include <stdint.h>
#include "timerCommon.h"
#include "ledCommon.h"

#if defined(STM32F103xB)
	#include "stm32f1xx.h"
#elif defined(STM32F429xx)
	#include "stm32f4xx.h"
#elif defined(STM32G071xx)
  #include "stm32g0xx.h"
#else
	#error "Not supported platform."
#endif


#include "main.h"
#include "boardLED.h"

BOARD_LED_CB	boardLeds[0] ;

static BOARD_LED_CB *bits_board_led_find_indexed ( BOARD_LED_FUNCTION func, unsigned int index )
{
	for( int i=0; i<sizeof(boardLeds) / sizeof(BOARD_LED_CB); i++)
	{
		if ( boardLeds[i].board_led_function == func &&
				boardLeds[i].board_led_function_ix == index ) return &boardLeds[i] ;
	}
	return NULL ;
}


static BOARD_LED_CB *bits_board_led_find ( BOARD_LED_FUNCTION func )
{
	return bits_board_led_find_indexed( func, 0 ) ;
}


void bits_board_led_assert ( BOARD_LED_CB *bdLedCb, bool state )
{
	bdLedCb->currentState = state ;
	HAL_GPIO_WritePin(
			bdLedCb->GPIOx,
			bdLedCb->GPIO_Pin,
			( state ) ? GPIO_PIN_SET : GPIO_PIN_RESET );
}


void bits_board_led_toggle ( BOARD_LED_CB *bdLedCb )
{
	bits_board_led_assert ( bdLedCb, ! bdLedCb->currentState ) ;
}


void bits_board_led_function_assert_indexed ( BOARD_LED_FUNCTION func, unsigned index, bool state )
{
	BOARD_LED_CB *ble = bits_board_led_find_indexed ( func, index ) ;
	if ( ble ) bits_board_led_assert( ble, state );
}


void bits_board_led_function_assert ( BOARD_LED_FUNCTION func,  bool state )
{
	BOARD_LED_CB *ble = bits_board_led_find ( func ) ;
	if ( ble ) bits_board_led_assert( ble, state );
}


void bits_board_led_heartbeat_toggle ( void )
{

}


// set msPulseTime to 0 if function not desired
//void bits_board_led_init ( LED_CB *ledCB, GPIO_TypeDef* GPIOx, unsigned int16_t GPIO_Pin, unsigned int16_t msPulseTime )
//{
//	ledCB->GPIO_Pin = GPIO_Pin ;
//	ledCB->GPIOx = GPIOx ;
//	ledCB->msPulseTime = msPulseTime ;
//}

void bits_board_led_pulse_indexed (
		BOARD_LED_FUNCTION func,
		unsigned int index,
		unsigned int durationMillisecs )
{

}


void bits_board_led_idle( unsigned int milliSecs )
{

}


void bits_board_led_init (void)
{
	// No LEDs in PCB v2
}

