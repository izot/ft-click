# FTMQ simple messaging protocol over Free Topology (FT)

## FTMQ

FTMQ is a simple messaging protocol to easily exchange data between nodes interconnected with MikroE **FT Click** modules that communicate using Adesto's Free Topology (FT) communication technology.
FTMQ aims to provide a very simple publish-subscribe communication model.

When a FTMQ packet is sent, is broadcasted to all nodes in the FT network.
Each recipient node filters the received messages and passes the wanted ones to the host application, based on message id.

## FTMQ Protocol Description

FTMQ is a simple, “MQTT-like” protocol, or API, for sending and receiving messages across Free Topology networks using any of the various FT Reference Design platforms.

The first available hardware adapter module that supports FTMQ is the “FT click” MikroBus compatible “Click Board” - a very popular standard started by MikroElektronika in Europe. It is expected that over time other adapters such as Grove, QUICC etc. may be added.

The FT click module can be plugged, either directly, or via a number of off-the-shelf adapter boards (“Shields”, “Hats”, “Capes”), into numerous development kits from e.g. MikroElectronika, ST Micro, Microchip, Raspberry-Pi and others.

The benefit of this approach is that the user gets to choose the development environment they are most familiar with, or the actual development toolchain that they will be using on their future product that include Adesto’s FT6050 chips.

FT click modules will support other protocol stacks, such as BACnet FT and LonWorks. However, this document will focus only on FTMQ protocol.

Initial support will be for:

Toolchains
ST-Micro STM32CubeIDE
Microchip - MPLab
Raspberry-Pi - gcc
Arduino - Arduino IDE

Dev Boards
MikroElektronika - Fusion V8 for ST-Micro
MikroElektronika - Clicker
ST-Micro - Nucleo-F429ZI
Microchip - TBD
Raspberry-Pi 4 Model B
Arduino

## FTMQ API reference

FTMQ messages are ‘free form’ packets of raw data sent using a ‘topic’ as a destination similar to MQTT. Unlike MQTT however, FTMQ does not require a broker. Each message is broadcast to all listening nodes. The listening node API layer filters out what packets it does not want, and submits the remaining packet to the user code with a simple callback.

Since the payload is binary, any data construction can be used, including simple ASCII, UTF-8 and jSON formatted messages. This choice is left to the user.

## Program flow

### Receiver
Initializes the API layer
“Subscribes” using familiar MQTT topic-style specifiers, and a callback function that is to be called if that topic is matched.
The user is responsible to call a tick function every few milliseconds to allow the API to process incoming messages
The callback function then processes any on-topic messages received.

```c
Void User_Callback ( byte *data, int *len )
{
    // .. parse and process received payload here
    // .. in our case, we know it is an ascii string
    printf (“Received: %s\n”, data ) ;
}

void main ( void )
{
    FTMQ_Init();

    FTMQ_Subscribe( “/listentopic/#”, User_Callback ) ;

    While ( true )
        {
        // … user code …
        FTMQ_Tick();
        Sleep ( 10 ) ; // milliseconds
        }
}
```

### Transmitter
Initializes the API layer
Builds a buffer to send
Calls a send function with a pointer to a topic, the data buffer and length

```c
void main ( void )
{
    char *sendpkt = “Hello World” ;

    FTMQ_Init();

    FTMQ_Publish(
        “/listentopic/subtopic”,
        sendpkt,
        strlen(sendpkt)+1 ) ;
}
```

Transmitting and receiving can occur from the same node, provided the FTPM_Tick() function gets called at regular intervals.

