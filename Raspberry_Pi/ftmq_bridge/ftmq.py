#****************************************************************************************
#
#   Copyright (C) 2020 ConnectEx, Inc.
#
#   This program is free software : you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this program.If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, if other files instantiate templates or
#   use macros or inline functions from this file, or you compile
#   this file and link it with other works to produce a work based
#   on this file, this file does not by itself cause the resulting
#   work to be covered by the GNU General Public License. However
#   the source code for this file must still be made available in
#   accordance with section (3) of the GNU General Public License.
#
#   This exception does not invalidate any other reasons why a work
#   based on this file might be covered by the GNU General Public
#   License.
#
#   For more information: info@connect-ex.com
#
#   For access to source code :
#
#       info@connect-ex.com
#           or
#       github.com/ConnectEx/BACnet-Dev-Kit
#
#***************************************************************************************


from ccp import CCP, Comm

class FTMQ:
    FTMQ_SEPARATOR = b'\x00'
    FTMQ_MAX_MSG = 49
    
    def __init__(self):
        self.ccp = CCP()
        self.ccp.register_callback(CCP.CCP_FTMQ_QUEUE, self.message_received)
        self.callbacks = []

    #This function is called each time a packet is received
    #it checks the topic and call the subscribed functions
    def message_received(self, msg):
        #print("received: ", msg)
        (topic, sep, payload) = msg.partition(self.FTMQ_SEPARATOR)
        try:
            topic_str = topic.decode()
            if (self.check_topic(topic_str) and len(payload) > 0):
                for callback in self.callbacks:
                    if topic_str == callback['topic']:
                        callback['callback'](topic_str, payload)
        except UnicodeError:
            # bad message
            pass 

    def publish(self,commid, topic,payload):
        msg = topic.encode() + self.FTMQ_SEPARATOR + payload
        #print(msg, len(msg))
        self.ccp.send_data(commid, CCP.CCP_FTMQ_QUEUE, msg)

    def subscribe(self, commid, r_topic, r_callback):
        self.callbacks.append(dict(topic=r_topic,callback=r_callback))
        # in python host handles topic filtering
        # send subscribe to all to ftclick (empty msg)
        #self.ccp.send_data(commid, CCP.CCP_FTMQ_QUEUE, bytes())
    
    def check_topic(self, topic):
        # check if topic contains illegal chars
        return True
