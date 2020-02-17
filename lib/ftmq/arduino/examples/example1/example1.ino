#include "ftmq.h"

/*
The PushButton click module xxxxx has a button and LED.
If you use the arduino mikrobus shield and plug
the pushbutton click on the mikrobus socket #2, the pinout will be:
#define BUTTON_PIN  3
#define LED_PIN  5
*/
#define BUTTON_PIN  3
#define LED_PIN  5

/*
Define some FTMQ ids for the network messages (FTMQ id is one byte length)
*/
#define BUTTON_ADDRESS  3
#define LED_ADDRESS  1

FTMQ ftmq;

/*
Create a callback to receive FTMQ messages
*/
void ledCallback(float value){
  if(value >= 1)
    analogWrite(LED_PIN, 128);
  else
    analogWrite(LED_PIN, 0);
}

void setup() {
  ftmq.begin();
  ftmq.subscribe(LED_ADDRESS, ledCallback);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (digitalRead(BUTTON_PIN)){
    float value = 1;
    ftmq.send(BUTTON_ADDRESS, value);
    delay(500);
  }
  // call this function periodically to attend FTMQ communications
  ftmq.loop();
}
