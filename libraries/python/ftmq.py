"""
FTMQ
"""

import struct

FTMQ_PKT_LEN = 5         # id + 4 byte data
HEADER_CHAR = 0x40

class FTMQ :

    def __init__(self) :
        self.read_buffer = bytearray()
        self.waiting_header = 0

    def publish(self, id, data):
        self.comm.write(HEADER_CHAR)
        self.comm.write(HEADER_CHAR)
        self.comm.write(id)
        self.comm.write(data)

    def start(self):
        data = struct.pack('<i', 0)
        self.publish(0,data)

    def poll(self):
        # parse all available data in the serial buffer
        data = self.comm.read(self.comm.in_waiting)
        for byte in data:
            self.parse(byte)

    def parse(self, byte):
        if self.waiting_header < 2:
            if byte == HEADER_CHAR:
                self.waiting_header += 1
        else:
            self.read_buffer.append(byte)
            if len(self.read_buffer) == FTMQ_PKT_LEN:
                print("receiving serial: ", self.read_buffer)
                id = self.read_buffer[0]
                value = self.read_buffer[1:]
                self.received_callback(id, value)
                self.read_buffer = bytearray() # cleanup
                self.waiting_header = 0

    def received_callback(self, id, data):
        pass
