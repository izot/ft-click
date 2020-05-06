#include "ccp_arduino.h"


// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillisMsec = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

uint8_t data[4] = { 'a', 'b', 'c', 'd'};
int serial_comm_id = 0;
int queue = 1;

void packet_received(uint8_t comm_id, uint8_t *data, int length) {

  Serial.print("len: ");
  Serial.println(length);
  Serial.write(data, 4);
}

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  CCP_Comm_HAL *Serial_comm = create_arduino_serial_comm();
  CCP_init();

  serial_comm_id = CCP_register_comm(Serial_comm);
  CCP_register_callback(1, packet_received);
  Serial.begin(115200);
  //Serial.println("hello CCP");
  CCP_sendPacket(serial_comm_id, queue, data, 4);

}

void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisMsec >= 1) {
    previousMillisMsec = currentMillis;
    CCP_poll_1msec();
  }
  if (currentMillis - previousMillis >= interval) {
    //CCP_sendPacket(serial_comm_id, queue, data, 4);
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }

}