#ifndef Lamp_h
#define Lamp_h

#include "Arduino.h"
#include <FastLED.h>

class Lamp {
  public:
    Lamp(int numSides, int ledsPerSide);
    void tick();
    void turnOn();
    void turnOff();
  private:
    int _numSides;
    int _ledsPerSide;
    int _updateFrequency = 1000;
    unsigned long _lastUpdate;
    int hue;
    int pos;
    CRGB * leds;

};

#endif
