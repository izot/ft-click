"""
FTMQ to MQTT bridge.
Connects a FTMQ based network to an MQTT broker.
Published messages in the FTMQ network are publised to the MQTT broker.
Received messages from the MQTT broker are sent to the FTMQ network.
A topic lookup table mapping MQTT topics to FTMQ identifier is needed, in yaml format.


Usage:     ftmq_mqtt_bridge.py [--broker <broker>] <FTMQ_port> <lookup>
           ftmq_mqtt_bridge.py (-h | --help)

Arguments:
    <FTMQ_port>              serial port to connect to FTMQ device
    <lookup>                file containing the topics lookup table

Options:
    -h --help               Show this screen
    --broker <broker>       Hostname of IP address of the MQTT broker [default: 127.0.0.1]
"""

from docopt import docopt

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish

from ccp import SerialComm
from ftmq import FTMQ

import time

import yaml
import json
import struct

import sys

POLL_WAIT_TIME = 0.001    # sleep time between serial polls. Avoids CPU usage on active waiting

class FTMQ_MQTT_Bridge :

    def __init__(self, broker, FTMQ_port, topics_lookup) :
        self.broker = broker
        self.FTMQ_port = FTMQ_port
        self.topics_lookup = topics_lookup
        self.ftmq = FTMQ()

    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        for topic in self.topics_lookup['mqtt_subscriptions']:
            self.client.subscribe(topic)

    def on_message(self, client, userdata, msg):
        print('on msg ' +msg.topic+" "+str(msg.payload))
        # publish to FTMQ
        for t in self.topics_lookup['lookup_table'].items():
            if t[0] == msg.topic:
                #print("match: {}".format(msg.topic))
                #print(t)
                ftmq_topic = t[1]['ftmq_topic']
                print("sizes: {} {} {}".format(len(ftmq_topic), len(msg.payload), FTMQ.FTMQ_MAX_MSG))
                if (len(ftmq_topic) + len(msg.payload) + 1) < FTMQ.FTMQ_MAX_MSG:
                    print("ftmq_pub")
                    self.ftmq.publish(self.commid, ftmq_topic, msg.payload)


    def generate_FTMQ_callbacks(self) :
        for i, t in enumerate(self.topics_lookup['lookup_table'].items()):
            name = "_ftmq_cb_{}".format(i)
            self.create_callback(name, t[0])
            self.ftmq.subscribe(self.commid, t[1]['ftmq_topic'], getattr(self, name))

    def create_callback(self, name, mqtt_topic) :
        def dyncallback(ftmq_topic, payload) :
            print("ftmq callback: ",ftmq_topic, payload.decode())
            self.client.publish(mqtt_topic, payload)
        setattr(self, name, dyncallback)


    def run(self) :
        # connect to FTMQ dev
        self.comm = SerialComm(self.FTMQ_port)
        self.commid = self.ftmq.ccp.register_comm(self.comm)
        # MQTT client
        self.client = mqtt.Client()
        # setup callbacks
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        # connect to MQTT broker
        self.client.connect(self.broker)
        self.generate_FTMQ_callbacks()
        # comunications loop
        self.client.loop_start()


        run_loop = True
        while run_loop:
            self.ftmq.ccp.poll_1msec()
            time.sleep(POLL_WAIT_TIME)

        self.client.loop_stop()




def main():
    args = docopt(__doc__)
    print(args)
    # parse topics lookup file
    with open(args['<lookup>'], 'r') as stream:
        try:
            topics_lookup = yaml.safe_load(stream)
            print(topics_lookup)
        except yaml.YAMLError as exc:
            print(exc)
        else:
            app = FTMQ_MQTT_Bridge(args['--broker'], args['<FTMQ_port>'], topics_lookup)
            app.run()

if __name__ == "__main__":
    main()
