import serial
from time import sleep
import sys
import struct


def send_float(port, address, fvalue):
    byts = struct.pack("f", float(fvalue))
    send_bytes(port, address, byts)


def send_bytes(port, address, payload):
    try:
        ser = serial.Serial(port, 115200, timeout=0.5)
    except serial.serialutil.SerialException:
        print("Couldn't open port {}".format(port))
        sys.exit(1)

    print("sending port {} ...".format(port))
    header = b'@@'

    address_bytes = bytearray()
    address_bytes.append(address)

    #address = b'\x0A'
    #payload = b'\x02\x03\x04\x05'

    all = header + address_bytes + payload
    ser.write(all)
    print("sent: {}".format(' '.join(str(x) for x in all)))

    ser.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(""" invalid options.
        ej:
        python activate_bootloader.py COM1""")
        sys.exit(1)
    
    port = sys.argv[1]
    address = 254
    send_bytes(port, address, b'\xAB\xCD\x0F\x57')
