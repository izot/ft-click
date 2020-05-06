#****************************************************************************************
#
#   Copyright (C) 2020 ConnectEx, Inc.
#
#   This program is free software : you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this program.If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, if other files instantiate templates or
#   use macros or inline functions from this file, or you compile
#   this file and link it with other works to produce a work based
#   on this file, this file does not by itself cause the resulting
#   work to be covered by the GNU General Public License. However
#   the source code for this file must still be made available in
#   accordance with section (3) of the GNU General Public License.
#
#   This exception does not invalidate any other reasons why a work
#   based on this file might be covered by the GNU General Public
#   License.
#
#   For more information: info@connect-ex.com
#
#   For access to source code :
#
#       info@connect-ex.com
#           or
#       github.com/ConnectEx/BACnet-Dev-Kit
#
#***************************************************************************************


import sys
from ccp import SerialComm
from ftmq import FTMQ
import time
import struct

def func1(topic, payload):
    '''Receives the subscribed topic and received payload'''
    print("function1")
    print("Topic:", end= " ")
    print(topic)
    print("Payload:", end= " ")
    print(payload.decode())

def temp(topic, payload):
    print("Topic:", end= " ")
    print(topic)
    print("Payload:", end= " ")
    #print(struct.unpack('<f', payload)[0])
    print(payload.decode())


if __name__ == "__main__":
    #Reads the serial port from the scripts arguments
    comm = SerialComm(sys.argv[1])
    #Creates a new FTMQ object
    ftmq = FTMQ()
    #Send the comm(serial, at least by now) to ftqm and return an id to this comm
    commid = ftmq.ccp.register_comm(comm)
    #subscribes to the specified topic, when a packet with this topic arrives the specified function will be called
    ftmq.subscribe(commid, "button1",func1)
    ftmq.subscribe(commid, "temperature",temp)
    print("Starting")
    time.sleep(1)
    ledstate = b'\x00'
    count  = 0
    while(True):
        count += 1
        #Do tasks like checking if there is any new incomming ftmq packet
        ftmq.ccp.poll_1msec()
        #sys.stdout.write(".")
        #sys.stdout.flush()
        if ((count % 1000)== 0):
            #Sends the specified data to the specifed topic using the commid device
            ftmq.publish(commid, "led1", ledstate)
            if (ledstate == b'\x01'):
                ledstate = b'\x00'
            elif (ledstate == b'\x00'):
                ledstate = b'\x01'
        time.sleep(0.001)
