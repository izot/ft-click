#include "ftmq.h"

FTMQ::FTMQ() {
}

void FTMQ::begin(){
    Serial.begin(115200);
    n_subscriptions = 0;
}

void FTMQ::loop(){
    if (Serial.available()<7)
        return;
    // Check header
    for(int i=0; i<2; i++){
        if (Serial.read() != '@')
            return;
    }
    protoFtmq p;
    p.address = Serial.read();
    for(int i=0; i<4; i++){
        p.valuebytes[i] = Serial.read();
    }
    // If this address is in sub_address, trigger the correspondent callback
    for(unsigned int i=0; i<n_subscriptions; i++){
        if(sub_address[i] == p.address){
            callbackFuncArray[i](p.value);
        }
    }
}

void FTMQ::subscribe(uint8_t address, callbackFunc func){
    sub_address[n_subscriptions] = address;
    callbackFuncArray[n_subscriptions] = func;
    n_subscriptions++;
}

void FTMQ::send(uint8_t address, float value) {
    protoFtmq p;
    p.address = address;
    p.value = value;
    Serial.print("@@");  // Header
    Serial.write(address);
    Serial.write(p.valuebytes[0]);
    Serial.write(p.valuebytes[1]);
    Serial.write(p.valuebytes[2]);
    Serial.write(p.valuebytes[3]);
}
