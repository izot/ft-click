#ifndef ftmq_h
#define ftmq_h

#include <avr/pgmspace.h>
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

const int MAX_SUSCTIPTIONS = 10;

struct protoFtmq
{
    uint8_t address;
    union
    {
        float   value;
        uint8_t valuebytes[4];
    };
};

class FTMQ {
    private :
        unsigned int n_subscriptions;
        uint8_t sub_address[MAX_SUSCTIPTIONS];
        void (*callbackFuncArray[MAX_SUSCTIPTIONS]) (float value);

    public:
        typedef void (*callbackFunc) (float arg1);

        FTMQ();
        void begin();
        void loop();
        void send(uint8_t address, float value);
        void subscribe(uint8_t address, callbackFunc func);
};

#endif	// ftmq_h
