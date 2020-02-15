# FTMQ_MQTT
Utility to connect a FTMQ based network to an MQTT based network.
- Messages received from the FTMQ network are published to the MQTT broker as long
as they have a matching entry in the topics lookup table.
- Messages received from the MQTT broker are sent to the FTMQ network as long
as they have a matching entry in the topics lookup table.
- Topics in MQTT are translated into FTMQ 256 bit identifiers.
## Requirements
Python 3.6+
pyyaml
pyserial
docopt
paho-mqtt

## Run
```
Usage:     ftmq_mqtt_bridge.py [--broker <broker>] <FTMQ_port> <baud_rate> <lookup>
           ftmq_mqtt_bridge.py (-h | --help)

Arguments:
    <FTMQ_port>              serial port to connect to FTMQ device
    <baud_rate>             serial port baud_rate
    <lookup>                file containing the topics lookup table

Options:
    -h --help               Show this screen
    --broker <broker>       Hostname of IP address of the MQTT broker [default: 127.0.0.1]
```
### Install python and pip
```
sudo apt install python3 python3-pip
```
### Without pipenv
- Install prerequisites
```
pip3 install --user pyyaml pyserial docopt paho-mqtt
```
- Run the utility:
```
python3 ftmq_mqtt_bridge.py "/dev/ttyS0" 115200 topics_lookup.yaml
```
### With pipenv
```
pip3 install --user pipenv
pipenv install
```
- Run the utility:
```
pipenv run python3 ftmq_mqtt_bridge.py "/dev/ttyS0" 115200 topics_lookup.yaml
```
## Topics lookup table file
The utility expects a *topics lookup table file* in yaml format:
```
lookup_table :
  location/light1 :
    id : 1
    name : led
    type : array

  location/sensor1 :
    id : 2
    name : temperature
    type : float
 mqtt_subscriptions :
   - location/sensor1
```
The first section of the file is the *lookup_table*, with one entry per each topic.
Each topic has associated an FTMQ *id*, a *name* to generate the json payload, and
a *type* to properly translate the binary FTMQ data.
Binary payloads have 4 bytes size. Available types are int32, float, or 4 byte array.
The last section, *mqtt_subscriptions*, is a list of topics to be subscribed on the
MQTT broker.
