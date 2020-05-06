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

#ifndef FTMQ_H
#define FTMQ_H
#include "stdint.h"

#define FTMQ_MAX_PACKET_LEN 49 // limit of LonSendMsg. to support bigger we have to handle fragmentation

//callback function pointers to be registeres to spe topics
typedef void (*FTMQ_receive_cb_t)(uint8_t *payload, uint16_t payload_length);

void FTMQ_init(void);
uint8_t FTMQ_publish(uint8_t commid, const char *topic, const uint8_t* payload, uint16_t payload_length);
uint8_t FTMQ_subscribe(uint8_t commid, const char *topic, FTMQ_receive_cb_t cb);
uint8_t FTMQ_sub_lookup(const char *topic);
uint8_t FTMQ_payload();
#endif
