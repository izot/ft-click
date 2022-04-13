# FT Click

The Dialog FT Click repository, containing documentation, hardware designs, and example code for using the MikroE FT Click to quickly create applications for the
Industrial Internet of Things that communicate using Dialog' Free Topology (FT) communication technology.


# Repository structure

## doc
FTMQ documentation.

## lib
FTMQ host libraries.
### ftmq
APIs for sending FTMQ packets and subscribing to FTMQ topics. FTMQ is a MQTT-like API. See the readme.md in this directory for more information on the API.
### ccp
Source code of the "Chip-to-Chip Protocol" that the various APIs and interfaces use to send and receive packets to the **FT click** module, and thereafter over the FT network.
### bacnet
A BACnet stack. This is uses to build the BACnet Server example project, and can be used for interfacing to Adesto's BACnet FT networks.
### util
Utility functions used by the libraries and examples.
### LON
#### LON
(Future) The LON API provides the ability to use the full LON stack on development systems connected to a LON FT network via the **FT Click** module.
#### ShortStack
(Future) The future ShortStack passthrough provides the ability use Adesto's ShortStack API, to communicate directly with the ShortStack MicroServer that is resident on the FT 6050 on the **FT Click** module

## examples
Code samples of **FT Click** module  usage with FTMQ messaging protocol
### BME860_Enviro
Using the BME860 Enviromental sensor (Temp, Atmospheric Pressure, Humidity, and IAC) this example reads the Enviromental click module and sends the four parameters to the listening nodes
### Pushbutton
This project broadcasts a FTMQ packet to all listening nodes over the FT network. Nodes that are listening on the appropriate topic will respond in their own ways
### RGB_LED
This project listens for pusbutton packets, and changes the RGB colors on the LED click board for each event
### NodeRED
A Raspberry Pi project that interfaces a **FT Click** module to the Raspberry Pi, creates and displays received parameters on the Node-RED dashboard
### DualLightstrip
Pushbutton control of LED light strips based on an Arduino platform
### BACnet
#### BACnet Server
A BACnet Server sample application.

## IDEs
### STM32cubeIDE
Project files for compiling the various projects in the examples folder in ST Microelectronics' IDE
### Visual Studio Code
Future projects for compiling the various projects in the examples folder under Visual Studio Code

