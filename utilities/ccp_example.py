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
import time
import random
from ccp import CCP, SerialComm

def func1(payload):
    '''Receives the payload of the received ccp packet'''
    print("function1")
    print("Payload:", end= " ")
    print(payload)

def func2(payload):
    print("function2")
    for b in bytes(payload):
        print(int(b), end=" ")

if __name__ == "__main__":
    #Reads the serial port from the scripts arguments
    comm = SerialComm(sys.argv[1])
    #Creates a new CCP object
    ccp = CCP()
    #Send the comm(serial, at least by now) to ccp and return and id to this comm
    commid = ccp.register_comm(comm)
    #Register the function that will be called when a new ccp packet arrives
    #The first argument is the queue
    ccp.register_callback(4,func1)
    ccp.register_callback(1,func2)
    print("Starting")
    while(True):
        #Do tasks like checking if there is any new incomming ccp packet
        ccp.poll_1msec()
        #Generates random data
        data = bytes(range(random.randrange(2,100)))
        #Sends the random data to the port especified at the start, to the queue number 1
        ccp.send_data(commid,1,data)
        #waits 1 second
        time.sleep(1.0)
