#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ftmq.h"
#include "debug.h"

UART_HandleTypeDef UART_FT;

uint8_t uartFtRxchar;
uint8_t uartFtRxMsg[5];

uint8_t n_subscriptions = 0;
uint8_t sub_address[MAX_SUSCTIPTIONS];


void protoFtmqSetup(){
	n_subscriptions = 0;
	HAL_UART_Receive_IT(&UART_FT, &uartFtRxchar, 1);
}


void  protoFtmqCheck(){
	static bool header1 = false;
	static bool header2 = false;

	if (header1 && header2){
	    struct protoFtmq p;
	    p.address = uartFtRxMsg[0];
	    for(uint8_t i=0; i<4; i++){
	    	p.valuebytes[i] = uartFtRxMsg[i+1];
	    }

		SERIAL_DEBUG_SPRINTF_5("\n\rGot message. Address %d\tbytes: %d, %d, %d, %d\r\n", uartFtRxMsg[0], uartFtRxMsg[1], uartFtRxMsg[2], uartFtRxMsg[3], uartFtRxMsg[4]);

		// If this address is in sub_address, trigger the correspondent callback
	    for(uint8_t i=0; i<n_subscriptions; i++){
	        if(sub_address[i] == p.address){
	            callbackFuncArray[i](p.value);
	        }
	    }

		header1=false;
		header2=false;
	}
	else if (uartFtRxchar=='@'){
		if (!header1){
			header1 = true;
		}
		else {
			header2=true;
			HAL_UART_Receive_IT(&UART_FT, uartFtRxMsg, 5);
			return;
		}
	}
	else{
		header1=false;
		header2=false;
	}
	HAL_UART_Receive_IT(&UART_FT, &uartFtRxchar, 1);
}

void protoFtmqsend(uint8_t address, float value){
    struct protoFtmq p;
    p.value = value;

    uint8_t tosend[7];
    tosend[0] = '@';
    tosend[1] = '@';
    tosend[2] = address;
    tosend[3] = p.valuebytes[0];
    tosend[4] = p.valuebytes[1];
    tosend[5] = p.valuebytes[2];
    tosend[6] = p.valuebytes[3];

    SERIAL_DEBUG_SPRINTF_2("Sending to address %d value %d\r\n",tosend[2], (int) p.value);
	HAL_UART_Transmit(&UART_FT, tosend,  sizeof(tosend), HAL_MAX_DELAY);
}


void protoFtmqSubscribe(uint8_t address, callbackFunc func){
    sub_address[n_subscriptions] = address;
    callbackFuncArray[n_subscriptions] = func;
    n_subscriptions++;
}
