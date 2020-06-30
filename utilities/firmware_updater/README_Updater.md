# Updater
With this tool you can flash a firmware to your FT Click
# Hookup
You will need a serial interface (usb-ttl or similar) connected to the FT Click

|Serial||FT_Click|
|------|----|--------|
|Tx|->|Rx|
|Rx|->|Tx|
|3.3v|->|3.3v|
|Gnd|->|Gnd|

# Usage
1. Plug your usb-ttl to the FT Click and the plug it to your computer
2. Open a terminal and write but **don't run** ``` Updater.exe COMX firmware.bin``` where COMX is the COM port asigned to your serial interface and firmware.bin is the firmware that you want to upload
3. Disconnect the 3.3v pin from the FT Click in order to reset the FT Click
3. Connect the 3.3v and in less than 10 seconds press Enter to excute the Updater that you prepared before in the step 2
4. If everything is okay you should see some info about the flash process. The last line MUST be ```Verification OK```, else check wiring or try to be quicker in the step 3