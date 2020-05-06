#include "ccp_arduino.h"
#include "ftmq_arduino.h"
#include <ArduinoJson.h>

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET


#define LED_COUNT 21 // Number of LEDs in White strip

#define NUMPIXELS 60 // Number of RGB LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5
//Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
Adafruit_DotStar RGBstrip(NUMPIXELS, DOTSTAR_BRG);


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    5


// Declare our NeoPixel strip object:
Adafruit_NeoPixel WHITEstrip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


// Runs 10 LEDs at a time along strip, cycling through red, green and blue.
// This requires about 200 mA for all the 'on' pixels + 1 mA per 'off' pixel.

int      head  = 0, tail = -10; // Index of first 'on' and 'off' pixels
uint32_t color = 0xFF0000;      // 'On' color (starts red)

int      whead  = 0, wtail = -10; // Index of first 'on' and 'off' pixels
uint32_t wcolor = 0xFF0000;      // 'On' color (starts red)


#define RGB_MESSAGE 13
#define WHITE_MESSAGE 12

void WhiteCallback(uint8_t *payload, uint16_t payload_length){

  StaticJsonDocument<49> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload, payload_length);

  // Test if parsing succeeds.
  if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.c_str());
    return;
  }
  wcolor =  doc["led"][0];
  wcolor = (wcolor<<8) | (uint8_t)doc["led"][1];
  wcolor = (wcolor<<8) | (uint8_t)doc["led"][2];
}

void RGBCallback(uint8_t *payload, uint16_t payload_length){

  StaticJsonDocument<49> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload, payload_length);

  // Test if parsing succeeds.
  if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.c_str());
    return;
  }
  color =  doc["led"][0];
  color = (color<<8) | (uint8_t)doc["led"][1];
  color = (color<<8) | (uint8_t)doc["led"][2];
}

int serial_comm_id = 0;

void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
// RGB
  RGBstrip.begin(); // Initialize pins for output
  RGBstrip.show();  // Turn all LEDs off ASAP
// WHITE

  WHITEstrip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  WHITEstrip.show();            // Turn OFF all pixels ASAP
  WHITEstrip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  CCP_Comm_HAL *Serial_comm = create_arduino_serial_comm();
  CCP_init();

  serial_comm_id = CCP_register_comm(Serial_comm);
  Serial.begin(115200);

  FTMQ_init();

  FTMQ_subscribe(serial_comm_id, "wstrip/command", WhiteCallback);
  FTMQ_subscribe(serial_comm_id, "RGBstrip/command", RGBCallback);
 

}

unsigned long previousMillisRGB = 0;    
unsigned long previousMillisWhite = 10;    
unsigned long previousMillisMsec = 0;    

const long interval = 20;  

void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisMsec >= 1) {
    previousMillisMsec = currentMillis;
    CCP_poll_1msec();
  }
  if (currentMillis - previousMillisRGB >= interval) {
    previousMillisRGB = currentMillis;
    refreshRGB();
  }
  if (currentMillis - previousMillisWhite >= interval) {
    previousMillisWhite = currentMillis;
    refreshWhite();
  }
}

void refreshRGB() {

  RGBstrip.setPixelColor(head, color); // 'On' pixel at head
  RGBstrip.setPixelColor(tail, 0);     // 'Off' pixel at tail
  RGBstrip.show();                     // Refresh strip

  if(++head >= NUMPIXELS) {         // Increment head index.  Off end of strip?
    head = 0;                       //  Yes, reset head index to start
  }
  if(++tail >= NUMPIXELS) tail = 0; // Increment, reset tail index
}

void refreshWhite() {

  WHITEstrip.setPixelColor(whead, wcolor); // 'On' pixel at head
  WHITEstrip.setPixelColor(wtail, 0);     // 'Off' pixel at tail
  WHITEstrip.show();                     // Refresh strip
 

  if(++whead >= LED_COUNT) {         // Increment head index.  Off end of strip?
    whead = 0;                       //  Yes, reset head index to start
  }
  if(++wtail >= LED_COUNT) wtail = 0; // Increment, reset tail index

}
