
# FTMQ
Simple pub sub protocol over FT

## Stack
```
User code
      |
   FTMQ user API
      |
Host FTMQ library
      |
    CCP API
      |
Host CCP library
      |
    CCP comm
      |
FTclick CCP library
      |
FTCLick FTMQ library
      |
    FT6050
```
## Publishing a message
User code --> FTMQ_pub(topic, payload)   -->  createFTMQ_packet(topic, payload)  -->  CCP_Send(FTMQ_packet)  ---

---------- (transmit from user platform to FTclick) -------------

--- CCP_receive_callback for FTMQ queue  --> broadcast FTMQ_packet over FT network

## Receiving a message
a broadcasted FTMQ message is received (FTclick) --> CCP_Send(FTMQ_packet) ---

---------- (transmit from FTclick to user platform) -------------

--- CCP_receive_callback for FTMQ queue ---> check received topic against subscription list --> call FTMQ_received_callback if match

## Subscribing to a topic
User code --> FTMQ_subscribe(topic)  --> store topic in topic subscription list 

The subscription list could be kept in the FTclick instead of the user platform, to offload the user platform from filtering incoming messages(TBD).
This could prove usefun in arduinos, with limited ram (arduino una has 2KB, FTclick has 32KB)

The receive procedure with the FTclick managing the subscriptions will be the following:

## Receiving a message
a broadcasted FTMQ message is received (FTclick) --> check received topic against subscription list --> CCP_Send(FTMQ_packet) if match ---

---------- (transmit from FTclick to user platform) -----------------

--- CCP_receive_callback for FTMQ queue ---> call FTMQ_received_callback

## Subscribing to a topic
User code --> FTMQ_subscribe(topic)  --> createFTMQ_packet(topic) --> CCP_Send(FTMQ_packet) ---

------------ (transmit from user platform to FTclick) -------------

--- CCP_receive_callback for FTMQ queue ---> store topic in topic subscription list 
