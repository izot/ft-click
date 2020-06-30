import sys
import subprocess

from activate_bootloader import send_bytes
from stm32loader import main
#first calls activate_bootloader
#then calls stm32loader -p port -b 115200 -e -w -v firmware

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(""" invalid options.
        ej:
        python bootloader_and_flash.py COM1 firmware.bin""")
        sys.exit(1)
    port = sys.argv[1]
    firmware = sys.argv[2]
    address = 254
    send_bytes(port, address, b'\xAB\xCD\x0F\x57')

    args = sys.argv[:]
    args[1] = "-p"
    args[2] = port
    args.append("-e")
    args.append("-w")
    args.append("-v")
    args.append(firmware)

    loader = main.Stm32Loader()
    loader.parse_arguments(args[1:])
    loader.connect()
    loader.read_device_id()
    loader.read_device_uid()
    loader.perform_commands()
