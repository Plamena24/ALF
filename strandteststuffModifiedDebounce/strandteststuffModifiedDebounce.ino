// Simple strand test for Adafruit Dot Star RGB LED strip.
// This is a basic diagnostic tool, NOT a graphics demo...helps confirm
// correct wiring and tests each pixel's ability to display red, green
// and blue and to forward data down the line.  By limiting the number
// and color of LEDs, it's reasonably safe to power a couple meters off
// the Arduino's 5V pin.  DON'T try that with other code!

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 280 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    11
#define CLOCKPIN   13
#define IN_WATER 0
#define NOT_IN_WATER 1

Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

typedef enum {
  AS_DARK,
  AS_BRIGHTENING,
  AS_DARKENING,
} ACTIVITY_STATE;

const uint32_t COLOR_BG = strip.Color(0,100,50);
const uint32_t COLOR_CHASE = strip.Color(150, 150, 150);
const uint32_t COLOR_TIP = strip.Color(225,225, 225);
//const int REFRESH_INTERVAL_MS = 10;
const int CHASE_LENGTH = 218;
const int END_LIGHT_INDEX = CHASE_LENGTH-1;
const int EFFECT_COUNT = 30;
uint8_t tip_status = NOT_IN_WATER;

struct {
  unsigned long nextRefreshMs = 0;
  bool isActive = false;
} S; // State

struct t_effect{
  int headIdx = -1;
  int tailIdx = -1;
};

t_effect activeEffects[EFFECT_COUNT] = {};

void setup() {
  pinMode(4, INPUT); // touch input
  
 // S.nextRefreshMs = millis();
  tip_status = NOT_IN_WATER;
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  for (int i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, 0x0);
  }
  strip.show();
}

// Runs 10 LEDs at a time along strip, cycling through red, green and blue.
// This requires about 200 mA for all the 'on' pixels + 1 mA per 'off' pixel.

void loop() {
  unsigned long currentTimeMs = millis();
 // if (currentTimeMs > S.nextRefreshMs) 
 // {
    // for debug
    //S.isActive = ((currentTimeMs / 1000) % 2);
    S.isActive = digitalRead(4);

    debounceSensorRead(S.isActive);
    updateTipLights(currentTimeMs);
    updateChaseLights();
    strip.show();                     // Refresh strip
 //   S.nextRefreshMs += REFRESH_INTERVAL_MS;
 // }
}

void debounceSensorRead(bool inWater) {
  switch(tip_status)
  {
    case IN_WATER:
      if (inWater) {
        tip_status = IN_WATER;  
      } 
      else {
        tip_status = NOT_IN_WATER;
      }
      break;
    case NOT_IN_WATER:
      if(inWater) {
        startChase();
        tip_status = IN_WATER;
      }
      else 
      {
        tip_status = NOT_IN_WATER;
      }
      break;
    default:
      break;
  }
}

void startChase() {  
  int i = 0;
  bool foundSlot = false;
  for (; i < EFFECT_COUNT; i++) {
    //find available slot
    if ((activeEffects[i].tailIdx < END_LIGHT_INDEX) & (activeEffects[i].tailIdx < 0)) 
    {
      foundSlot = true;
      break;
    } 
  }
    //set values for new chase sequence
  if (foundSlot)
  {
    activeEffects[i].headIdx = END_LIGHT_INDEX;
    activeEffects[i].tailIdx = END_LIGHT_INDEX + 10;
  }
}

void updateChaseLights() {
  int i = 0;
  for (int i; i < EFFECT_COUNT; i++) {
    if (activeEffects[i].headIdx >= 0) {
      if (activeEffects[i].headIdx <= END_LIGHT_INDEX) {
        strip.setPixelColor(activeEffects[i].headIdx, COLOR_CHASE); // 'On' pixel at head
      }
      activeEffects[i].headIdx--;
 
    }
    if (activeEffects[i].tailIdx >= 0) {
      if (activeEffects[i].tailIdx <= END_LIGHT_INDEX) {
        strip.setPixelColor(activeEffects[i].tailIdx, COLOR_BG); // 'Off' pixel at tail      
      }
      activeEffects[i].tailIdx--;
    }
  }
}

void updateTipLights(unsigned long t) {
  uint32_t color = 0x0;
  if (S.isActive) {
    color = COLOR_TIP;
  }
  for (int i=CHASE_LENGTH+1; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, color);
  }
}

