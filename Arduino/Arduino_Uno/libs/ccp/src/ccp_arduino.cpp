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

#include "ccp_arduino.h"

CCP_Comm_HAL comm;

void arduino_serial_init() {

    Serial.begin(115200);
}

void arduino_serial_start() {

}

void arduino_serial_stop() {

}

void arduino_serial_poll() {

}

void arduino_serial_send_bytes(uint8_t *data, uint16_t length) {

    Serial.write(data, length);
}

void arduino_serial_read_bytes(uint8_t *data, uint16_t length) {

    Serial.readBytes(data,length);
}

int arduino_serial_has_bytes() {

    return Serial.available();
}

CCP_Comm_HAL *create_arduino_serial_comm() {
  comm.init = arduino_serial_init;
  comm.start = arduino_serial_start;
  comm.stop = arduino_serial_stop;
  comm.poll = arduino_serial_poll;
  comm.send_bytes = arduino_serial_send_bytes;
  comm.read_bytes = arduino_serial_read_bytes;
  comm.has_bytes = arduino_serial_has_bytes;

  return(&comm);
}
