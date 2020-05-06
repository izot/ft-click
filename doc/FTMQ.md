# FTMQ simple messaging protocol over Free Topology (FT)

## FTMQ

FTMQ is a simple messaging protocol to easily exchange data between nodes interconnected with MikroE **FT Click** modules that communicate using Adesto's Free Topology (FT) communication technology.
FTMQ aims to provide a very simple publish-subscribe communication model.

### FTMQ packet

FTMQ packets consist on a variable length packets formed by topic and payload

| topic | data |
| :---- | :--- |

When a FTMQ packet is sent, is broadcasted to all nodes in the FT network.
Each recipient node filters the received messages and passes the wanted ones to the host application, based on topic.

### API usage
- FTMQ Publish:
    Sends specified payload to specified topic
- FTMQ Subscribe:
    Subscribes the host to an specifed topic, when a packet to this topic arrives a callback funcion will be executed. 
