#ifndef Lamp_h
#define Lamp_h

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

class Lamp {
  public:
    Lamp(int pin, int numSides, int ledsPerSide);
    void turnOn();
  private:
    int _pin;
    int _numSides;
    int _ledsPerSide;
    Adafruit_NeoPixel strip;
};

#endif
