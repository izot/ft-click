# FTMQ simple messaging protocol over Free Topology (FT)

## FTMQ

FTMQ is a simple messaging protocol to easily exchange data between nodes interconnected with MikroE **FT Click** modules that communicate using Adesto's Free Topology (FT) communication technology.
FTMQ aims to provide a very simple publish - subscribe communication model.

### Roadmap
- FTMQ v0.2: Provides a basic functionality to comunicate through a FT network.
It consists on small fixed length packets with an integer identifier and 4 byte data.
The identifier is the *"topic"*, so it identifies the data, not the node sending it. There is no topic registration mechanism, so each node needs to know in advance the topics(ids) map, to publish or listen to the relevant messages.
- FTMQ v1.0: Provides variable length packets, text based topics and binary or text payloads.

### FTMQ packet

FTMQ packets consist on 7 byte packets

| id | data | data | data | data |
| :- | :--- | :--- | :--- | :--- |

When a FTMQ packet is sent, is broadcasted to all nodes in the FT network.
Each recipient node filters the received messages and passes the wanted ones to the host application, based on message id.

### Special packets
- FTMQ startup packet
- jump to bootloader packet

### API usage
- FTMQ send
- FTMQ receive
