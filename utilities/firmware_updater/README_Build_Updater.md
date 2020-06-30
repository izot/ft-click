# Updater Building Instructions
This file describe the requirements and instructions to build the Updater.exe.
## Requirementes
* Windows 7 or newer
* python 3.6 or superior
* pip 19.0 or superior
* pyinstaller 3.6
## How to install pyinstaller
Assuming that you already have python and pip working

```pip install pyinstaller```
## How to generate the Updater.exe
Open a new shell and go to ftrd-software/FT_Click/tools\boot_and_flash and run the next command :

```pyinstaller.exe --onefile Updater.py```

The generated Updater.exe is located in the dist folder
