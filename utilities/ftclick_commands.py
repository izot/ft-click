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


"""FTClick commands.
Utility to issue FTClick special commands.

Usage:     ftclick_commands.py <port> <command> [<args>...]
           ftclick_commands.py (-h | --help)

Arguments:
    <port>              serial port to connect to FTClick device

Commands:
    loopback            sends back any received ccp packet
    burst               activate sending continuous dummy packets over the FT network, for stress test
    lonreset            send a reset command to the shortstack microserver
    neuron-reset-pin    activate or deactivate neuron reset pin (hardware reset)
    en-debug-q          enable debug queue
    dis-debug-q         disable debug queue
    hw-ver              query FTClick hardware version
    sw-ver              query FTClick software version
    nodeid-set          set neuron node id
    nodeid-get          query neuron node id

Options:
    -h --help               Show this screen

"""

subcommands = { 
'loopback'         :    'Usage:     loopback (on|off)',
'burst'            :    'Usage:     burst (on|off)',
'lonreset'         :    'Usage:     lonreset',
'neuron-reset-pin' :    'Usage:     neuron-reset-pin  (on|off)',
'en-debug-q'       :    'Usage:     en-debug-q (lon|ftmq|bacnet)',
'dis-debug-q'      :    'Usage:     dis-debug-q (lon|ftmq|bacnet)',
'hw-ver'           :    'Usage:     hw-ver',
'sw-ver'           :    'Usage:     sw-ver',
'nodeid-set'       :    'Usage:     nodeid-set <id>',
'nodeid-get'       :    'Usage:     nodeid-get',
}

from docopt import docopt

import sys
import time
from ccp import CCP, SerialComm

class CCP_Command:
    COMMAND_QUEUE = 4
    COMMAND_RES_TIMEOUT = 1.0
    COMMANDS = { 'loopback' : 0,
                 'lonreset' : 2,
                 'neuron-reset-pin' : 3,
                 'en-debug-q' : 4,
                 'dis-debug-q' : 5,
                 'hw-ver' : 6,
                 'sw-ver' : 7,
                 'nodeid-set' : 8,
                 'nodeid-get' : 1,
                 'burst' : 9 }

    DEBUG_QUEUES = {'lon' : 0,
                    'ftmq': 1,
                    'bacnet': 3 }

    def __init__(self, ccp, commid):
        self.ccp = ccp
        self.commid = commid
        self.ccp.register_callback(self.COMMAND_QUEUE, self.command_cb)

    def send_command(self, command, **kwargs):
        command_id = self.COMMANDS[command]
        command_data = 0
        if ('on' in kwargs and kwargs['on']):
            command_data = 1
        else:
            command_data = 0
        if ('<id>' in kwargs and kwargs['<id>']):
            command_data = int(kwargs['<id>'])
        if (any(o in kwargs for o in ['lon', 'ftmq', 'bacnet'])):
            for key, value in kwargs.items():
                if (value):
                    command_data = self.DEBUG_QUEUES[key]
        data = bytes([command_id, command_data])
        self.ccp.send_data(commid, self.COMMAND_QUEUE, data)

    def command_cb(self, payload):
        print("command: {} ".format(str(payload[0])))
        for b in bytes(payload):
            print(int(b), end=" ")


if __name__ == "__main__":
    args = docopt(__doc__)
    #print(args)

    comm = SerialComm(args['<port>'])
    ccp = CCP()
    commid = ccp.register_comm(comm)
    ccp_command_q = CCP_Command(ccp, commid)

    command_parsed_args = docopt(subcommands[args['<command>']], argv=args['<args>'])
    #print(command_parsed_args)
    ccp_command_q.send_command(args['<command>'], **command_parsed_args)

    # wait for a response
    start_time = time.time()
    while((time.time() - start_time) < CCP_Command.COMMAND_RES_TIMEOUT):
        ccp.poll_1msec()
        time.sleep(0.001)
