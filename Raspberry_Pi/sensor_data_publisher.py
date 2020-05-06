"""
Sensor data publisher.
bme680 sensor data generator
if multiple topics are provided, different sensor data will be published to all of them.

Usage:     sensor_data_publisher.py [--broker <broker>] [--rate <rate>]
           sensor_data_publisher.py (-h | --help)

Arguments:
    <topic>                 MQTT topic to publish sensor data

Options:
    -h --help               Show this screen
    --broker <broker>       Hostname of IP address of the MQTT broker [default: 127.0.0.1]
    --rate <rate>           Sensor data publishing rate (in seconds) [default: 1]
"""

from docopt import docopt

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish

import time

import yaml
import sys
import json
import random

class SensorDataPublisher:

    def __init__(self, broker, rate) :
        self.broker = broker
        self.sensor_rate = int(rate)

    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code "+str(rc))

    def run(self) :

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.connect(self.broker, 1883,60)

        self.client.loop_start()

        run_loop = True
        while run_loop:
            self.client.publish('building/hall/temperature', payload=json.dumps({"temperature" : round(random.random(), 1) + 25.0}))
            self.client.publish('building/hall/humidity', payload=json.dumps({"humidity" : round(random.random(), 1) + 60.0}))
            self.client.publish('building/hall/pressure', payload=json.dumps({"pressure" : round(random.random(), 1) + 1013.0}))
            self.client.publish('building/hall/AQI', payload=json.dumps({"AQI" : round(random.random(), 1) + 10.0}))
            time.sleep(self.sensor_rate)

        self.client.loop_stop()

    def generate_data(self):
        data = {'temperature' : 25.0, 'humidity' : 60.0, 'pressure' : 1013.0, 'VOC' : 10.0}
        data['temperature'] += round(random.random(), 1)
        data['humidity'] += round(random.random(), 1)
        data['pressure'] += round(random.random(), 1)
        data['VOC'] += round(random.random(), 1)
        return json.dumps(data)



def main():
    args = docopt(__doc__)
    print(args)
    app = SensorDataPublisher(args['--broker'], args['--rate'])
    app.run()

if __name__ == "__main__":
    main()
