/****************************************************************************************
*
*   Copyright (C) 2020 ConnectEx, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   As a special exception, if other files instantiate templates or
*   use macros or inline functions from this file, or you compile
*   this file and link it with other works to produce a work based
*   on this file, this file does not by itself cause the resulting
*   work to be covered by the GNU General Public License. However
*   the source code for this file must still be made available in
*   accordance with section (3) of the GNU General Public License.
*
*   This exception does not invalidate any other reasons why a work
*   based on this file might be covered by the GNU General Public
*   License.
*
*   For more information: info@connect-ex.com
*
*   For access to source code :
*
*       info@connect-ex.com
*           or
*       github.com/ConnectEx/BACnet-Dev-Kit
*
****************************************************************************************/

#include "ftmq_config.h"
#include "ftmq.h"
#include "ccp.h"

#include "string.h"

// ---------------- CONSTANTS --------------------------------
#define FTMQ_SEPARATOR 0x00
#define FTMQ_START_PRINTABLE_CHARACTER 32
#define FTMQ_END_PRINTABLE_CHARACTER 126

// -------------- CUSTOM TYPES ---------------------------------
/*
typedef struct FTMQ_Packet {
    uint8_t topic[FTMQ_MAX_TOPIC_LEN];
    uint8_t separator;
    uint8_t payload[FTMQ_MAX_DATA_LEN];
} FTMQ_Packet;
*/
typedef struct FTMQ_receive_callback {
    FTMQ_receive_cb_t receive;
    uint8_t msg[FTMQ_MAX_PACKET_LEN];
    uint8_t topic_length;
} FTMQ_receive_callback;
/*
typedef struct FTMQ_Comm {

} FTMQ_Comm;
*/
// ------------ PRIVATE FUNCTION PROTOTYPES ---------------------------------
/*uint8_t check_ftmq_params(const char *topic, const char *data);
uint8_t check_ftmq_topic(const char *topic);
uint8_t check_ftmq_data(const char *data);
uint8_t ftmq_packet_from_payload(char *data, uint8_t length, FTMQ_Packet *packet);
uint8_t generate_ftmq_packet(const char *topic,const char *data, FTMQ_Packet *packet);
*/
void manage_callbacks(uint8_t commid, uint8_t *data, int length);

// ------------- LIBRARY GLOBAL VARIABLES ----------------------------
#ifdef FTMQ_MAX_SUBSCRIPTIONS  // this host handles subscriptions
uint8_t registered_FTMQ_callbacks = 0;
FTMQ_receive_callback FTMQ_callbacks[FTMQ_MAX_SUBSCRIPTIONS];
#else // FTclick handles the subscriptions

#endif

uint8_t out_buffer[FTMQ_MAX_PACKET_LEN];

// ------------ PUBLIC FUNCTIONS -------------------------------------
// TODO que debe recibir esta función? un puntero a ccp? debe inicializar directamente el ccp sin recibir parámetros?
// MT: Al incluir ccp.h, todos los recursos del ccp estan presentes. Como no es un objeto ni es dinamico, simplemente
// lo usamos tal cual. El init del ccp debe hacerse fuera, ya que el usuario podria usarlo para mas cosas que el ftmq
void FTMQ_init() {
    CCP_register_callback(CCP_FTMQ_QUEUE, manage_callbacks);
}

uint8_t FTMQ_publish(uint8_t commid, const char *topic, const uint8_t* payload, uint16_t payload_length) {
    uint8_t topic_length = strlen(topic);
    memcpy(out_buffer, topic, topic_length); 
    out_buffer[topic_length] = 0;// include the null terminator
    memcpy(out_buffer + topic_length + 1, payload, payload_length);
    CCP_sendPacket(commid, CCP_FTMQ_QUEUE, out_buffer, topic_length + 1 + payload_length);
}

//TODO test
uint8_t FTMQ_subscribe(uint8_t commid, const char *topic, FTMQ_receive_cb_t cb){
    if (registered_FTMQ_callbacks < FTMQ_MAX_SUBSCRIPTIONS){
        FTMQ_callbacks[registered_FTMQ_callbacks].receive = cb;
        FTMQ_callbacks[registered_FTMQ_callbacks].topic_length = strlen(topic);
        memcpy(FTMQ_callbacks[registered_FTMQ_callbacks].msg, topic, FTMQ_callbacks[registered_FTMQ_callbacks].topic_length + 1);
        registered_FTMQ_callbacks++;
    }
}


void manage_callbacks(uint8_t commid, uint8_t *data, int length){
#ifdef FTMQ_MAX_SUBSCRIPTIONS
    for (uint8_t i = 0; i < registered_FTMQ_callbacks;i++){
        if (strncmp(data, FTMQ_callbacks[i].msg, FTMQ_callbacks[i].topic_length) == 0){
            FTMQ_callbacks[i].receive(data + FTMQ_callbacks[i].topic_length + 1, length - (FTMQ_callbacks[i].topic_length + 1));
        }
    }
#else

#endif
}
/*
uint8_t ftmq_packet_from_payload(char *data, uint8_t length, FTMQ_Packet *packet){
    for (uint8_t i = 0; i < length; i++){
        if (data[i] == FTMQ_SEPARATOR){
            memcpy(packet->topic,data,i);
            memcpy(packet->payload,data+i+1,length-i-1);
            return 0;
        }
    }
    return 1;
}

//checks if topic is a valid topic
//TODO test
uint8_t check_ftmq_topic(const char *topic){

    //check if topic is a null-terminated string
    if( strnlen(topic,FTMQ_MAX_TOPIC_LEN) == FTMQ_MAX_TOPIC_LEN ){
        return 1;
    }

    //check if topic is formed only by printable characters
    for(uint8_t i = 0; i< strlen(topic); i++){
        if (topic[i] < FTMQ_START_PRINTABLE_CHARACTER || topic[i] > FTMQ_END_PRINTABLE_CHARACTER) {
            return 2;
        }
    }
    return 0;
}

//TODO test
uint8_t check_ftmq_data(const char *data){

    //check if data is a null-terminated string
    if ( strnlen(data,FTMQ_MAX_DATA_LEN) == FTMQ_MAX_DATA_LEN ){
        return 3;
    }
    //check if data is formed only by printable characters
    for(uint8_t i = 0; i< strlen(data); i++){
        if (data[i] < FTMQ_START_PRINTABLE_CHARACTER || data[i] > FTMQ_END_PRINTABLE_CHARACTER) {
            return 4;
        }
    }
    return 0;
}

//TODO test
uint8_t check_ftmq_params(const char *topic, const char *data){

    uint8_t result;
    //check if topic and data are null terminated strings and
    //are formed only by printable chracters
    result = check_ftmq_topic(topic);
    if (result != 0){
        return result;
    }
    result = check_ftmq_data(data);
    if (result != 0){
        return result;
    }

    //check if topic and data are bigger than MAX_PACKET_LENGTH
    if (strlen(topic) + strlen(data) > FTMQ_MAX_PACKET_LEN){
        return 5;
    }

   return 0;
}

*/
