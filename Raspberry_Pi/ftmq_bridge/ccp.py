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


import enum
import time
import serial
import struct


class CCP:

    CCP_PASSTHROUGH_QUEUE = 0
    CCP_FTMQ_QUEUE = 1
    CCP_DEBUG_QUEUE = 2
    CCP_BACNET_QUEUE = 3
    CCP_COMMAND_QUEUE = 4

    CCP_STATES = enum.Enum('STATES', 'IDLE PREAMBLE HEADER DATA CRC')
    CCP_HEADER_LEN = 3
    CCP_CRC_LEN = 2
    CCP_TIMEOUT = 1000
    CCP_MAX_PAYLOAD = 189
    CCP_PREAMBLE = b'@@'
    CCP_PREAMBLE_LEN = 2
    CCP_OVERHEAD_LEN = CCP_HEADER_LEN + CCP_CRC_LEN + CCP_PREAMBLE_LEN
    CCP_MAX_PACKET = CCP_PREAMBLE_LEN + CCP_HEADER_LEN + CCP_MAX_PAYLOAD + CCP_CRC_LEN

    def __init__(self):
        self.comms = []
        self.callbacks = []

    def register_comm(self,comm):
        '''
        Adds a new comm to the CCP object, each object can handle multiple
        comms at the same time
        '''
        self.comms.append(comm)
        self.comms[-1].start_comm()
        return len(self.comms)-1
        
    def register_callback(self,r_queue, r_callback):
        '''
        Register a new callback function to the specified queue.
        Any message that arrives to this queue from any of the registered comms
        will trigger the callback function
        '''
        self.callbacks.append(dict(queue=r_queue,callback=r_callback))

    def send_data(self, comm_id, queue, data: bytes):
        '''
        Builds a valid ccp packet from the data argument and queue argument
        and sends it using the specified comm device (identified by the comm_id
        '''
        length = (len(data) + self.CCP_OVERHEAD_LEN).to_bytes(2, 'little')
        header = length + (queue).to_bytes(1, 'little')
        packet = self.CCP_PREAMBLE + header + data
        crc = self.crc16(packet)
        packet = packet + (crc).to_bytes(2, 'little')
        self.comms[comm_id].send_bytes(packet)

    def poll_1msec(self):
        '''
        This function should be called each milisecond.
        Updates timeouts,and reads any incomming byte, the parse it
        It exits and discards the incomming packet if the timeout is reached
        '''

        for i, comm in enumerate(self.comms):
            comm.tick()
            comm.poll()
            if comm.has_bytes():
                data = comm.read_bytes()
                for b in data:
                    self.parse_byte(b, comm)
                comm.restore_timeout()

    def parse_byte(self,b, comm):
        '''
        State machine that parses only one byte at each run
        This will hanlde all defined states and exits if timeout is reached
        It also checks if the packets had a valid checksum, an finally will
        call the registered callback functions to this queue when the packet
        is complety received.

        Note tha this will call all function registered to a queue no matter
        the comm device that received it.
        '''
        if comm.state == self.CCP_STATES.IDLE:
            if b == self.CCP_PREAMBLE[0]:
                comm.state = self.CCP_STATES.PREAMBLE
                comm.preamble = bytearray()
                comm.header = bytearray()
                comm.data = bytearray()
                comm.crc = bytearray()
                comm.preamble.append(b)

        elif comm.state == self.CCP_STATES.PREAMBLE:
            comm.preamble.append(b)
            if (len(comm.preamble) == self.CCP_PREAMBLE_LEN):
                if (comm.preamble == self.CCP_PREAMBLE):
                    comm.state = self.CCP_STATES.HEADER
                else:
                    comm.state = self.CCP_STATES.IDLE

        elif comm.state == self.CCP_STATES.HEADER:
            comm.header.append(b)
            if (len(comm.header) == (self.CCP_HEADER_LEN - 1)):
                comm.length = int.from_bytes(comm.header, 'little') 
                if (comm.length <= self.CCP_OVERHEAD_LEN):
                    comm.state = self.CCP_STATES.IDLE
                if (comm.length > (self.CCP_MAX_PACKET - self.CCP_PREAMBLE_LEN)):
                    comm.state = self.CCP_STATES.IDLE
            if (len(comm.header) == self.CCP_HEADER_LEN):
                comm.queue = b
                comm.state = self.CCP_STATES.DATA

        elif comm.state == self.CCP_STATES.DATA:
            comm.data.append(b)
            if len(comm.data) == (comm.length - self.CCP_OVERHEAD_LEN):
                comm.state = self.CCP_STATES.CRC

        elif comm.state == self.CCP_STATES.CRC:
            comm.crc.append(b)
            if (len(comm.crc) == self.CCP_CRC_LEN):
                in_crc = self.crc16(self.CCP_PREAMBLE + comm.header + comm.data).to_bytes(2, 'little')
                if (in_crc != comm.crc):
                    #clean and return to idle
                    comm.state = self.CCP_STATES.IDLE
                else:
                    #call callback functions
                    for callback in self.callbacks:
                        if callback['queue'] == comm.queue:
                            callback['callback'](comm.data)
                            comm.state = self.CCP_STATES.IDLE


    @staticmethod
    def crc16( nData: bytes):
        ''' Returns the crc16 checksum of nDatad'''
        INITIAL_MODBUS = 0xFFFF
        
        table = (
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 )

        crc = INITIAL_MODBUS

        for ch in nData:
            crc = (crc >> 8) ^ table[(crc ^ ch) & 0xFF]
        return crc


class Comm:
    '''Generic comm class from where other should inherit'''
    def __init__(self):
        self.timeout = 0
        self.state = CCP.CCP_STATES.IDLE
        
    def start_comm(self):
        ''' Do the needed steps to initialice the comm device'''
        pass

    def stop_comm(self):
        ''' Do the needed steps to stop the comm device'''
        pass

    def send_bytes(self, b):
        ''' Sends the bytes received as argument to the comm device'''
        pass

    def read_bytes(self):
        '''Reads any received byte from the comm device'''
        pass

    def has_bytes(self):
        '''Returns the number of pendign bytes to be readed'''
        pass

    def poll(self):
        pass

    def tick(self):
        '''Updates timeout'''
        self.timeout -= 1
        if (self.timeout < 0):
            self.timeout = 0
        if (self.timeout == 0):
            self.state = CCP.CCP_STATES.IDLE

    def restore_timeout(self):
        '''Resets timeout'''
        self.timeout = CCP.CCP_TIMEOUT

class SerialComm(Comm):
    '''
    This class is inherited from comm class abstraction
    for the Serial comm devices
    '''
    

    def __init__(self,port):
        self.port = port
        super(SerialComm, self).__init__()

    def start_comm(self):
        ''' Starts the serial comm at prefixed baudrate'''
        self.serial_port = serial.Serial(self.port, 115200, timeout = 0.1, write_timeout = 0.1)

    def stop_comm(self):
        ''' Stop the serial comm'''
        self.serial_port.__del__()
        pass

    def send_bytes(self, b):
        '''Sends the argument b to the serial port'''
        self.serial_port.write(b)

    def read_bytes(self):
        '''Returns the bytes received by the serial comm device'''
        return self.serial_port.read(self.serial_port.in_waiting)

    def has_bytes(self):
        '''Returns the number of bytes that are waiting to be readed from the serial comm device'''
        return self.serial_port.in_waiting
