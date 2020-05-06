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

#include "ccp_stm32.h"
#include "common.h"


uint8_t mikrobus_rx_buff[MIKROBUS_RX_BUFF_SIZE];
uint8_t mikrobus_rx_buff_head = 0;
uint8_t mikrobus_rx_buff_tail = 0;

void stm32_receive_IT() {
	uint8_t next_tail = (mikrobus_rx_buff_tail + 1) % MIKROBUS_RX_BUFF_SIZE;
	if (next_tail != mikrobus_rx_buff_head) {// free space in the buffer
		mikrobus_rx_buff[mikrobus_rx_buff_tail] = uartcRxchar;
		mikrobus_rx_buff_tail = (mikrobus_rx_buff_tail + 1) % MIKROBUS_RX_BUFF_SIZE;
	}
	HAL_UART_Receive_IT(&CLICK_UART, &uartcRxchar, 1);
}

void stm32_serial_init() {

}

void stm32_serial_start() {

}

void stm32_serial_stop() {

}

void stm32_serial_poll() {
}

void stm32_serial_send_bytes(uint8_t *data, uint16_t length) {

	HAL_UART_Transmit_IT(&CLICK_UART, data,  length);
}

void stm32_serial_read_bytes(uint8_t *data, uint16_t length) {

	if (mikrobus_rx_buff_head > mikrobus_rx_buff_tail) {
		if ((mikrobus_rx_buff_head + length) < MIKROBUS_RX_BUFF_SIZE) {
			memcpy(data, mikrobus_rx_buff + mikrobus_rx_buff_head, length);
			mikrobus_rx_buff_head += length;
		} else {
			uint16_t first_block = MIKROBUS_RX_BUFF_SIZE - mikrobus_rx_buff_head;
			uint16_t second_block = length - first_block;
			memcpy(data, mikrobus_rx_buff + mikrobus_rx_buff_head, first_block );
			memcpy(data, mikrobus_rx_buff, second_block );
			mikrobus_rx_buff_head = second_block;
		}
	} else {
		memcpy(data, mikrobus_rx_buff + mikrobus_rx_buff_head, length);
		mikrobus_rx_buff_head += length;
	}
}

int stm32_serial_has_bytes() {

	if (mikrobus_rx_buff_head != mikrobus_rx_buff_tail) {
		if (mikrobus_rx_buff_head > mikrobus_rx_buff_tail)
			return (MIKROBUS_RX_BUFF_SIZE - mikrobus_rx_buff_head) + (mikrobus_rx_buff_tail);
		if (mikrobus_rx_buff_head < mikrobus_rx_buff_tail)
			return mikrobus_rx_buff_tail - mikrobus_rx_buff_head;
	}
	else
		return 0;
}

CCP_Comm_HAL *create_stm32_serial_comm(CCP_Comm_HAL *comm) {
  comm->init = stm32_serial_init;
  comm->start = stm32_serial_start;
  comm->stop = stm32_serial_stop;
  comm->poll = stm32_serial_poll;
  comm->send_bytes = stm32_serial_send_bytes;
  comm->read_bytes = stm32_serial_read_bytes;
  comm->has_bytes = stm32_serial_has_bytes;

  return(comm);
}
