#include "ccp_arduino.h"
#include "ftmq_arduino.h"

#define BUTTON_PIN  3
#define LED_PIN  LED_BUILTIN


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        
int buttonInterval = 100;
unsigned long previousMillisMsec = 0;        

int serial_comm_id = 0;


void led_msg_received(uint8_t *payload, uint16_t payload_length) {

  if (payload[0] != 0)
    digitalWrite(LED_PIN, LOW);
  else   
    digitalWrite(LED_PIN, HIGH);
}

void setup() {
  // set the digital pin as output:
  CCP_Comm_HAL *Serial_comm = create_arduino_serial_comm();
  CCP_init();

  serial_comm_id = CCP_register_comm(Serial_comm);
  Serial.begin(115200);

  FTMQ_init();

  FTMQ_subscribe(serial_comm_id, "led1", led_msg_received);
  
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}


void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= buttonInterval) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      FTMQ_publish(serial_comm_id, "button1", "1", 1);
      previousMillis = currentMillis;
    }
  }

  if (currentMillis - previousMillisMsec >= 1) {
    previousMillisMsec = currentMillis;
    CCP_poll_1msec();
  }

}