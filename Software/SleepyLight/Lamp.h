#ifndef Lamp_h
#define Lamp_h

#include "Arduino.h"


#include <FastLED.h>

enum LampMode {
  SOLID,
  COOLDOWN
};

class Lamp {
  public:
    Lamp(int numSides, int ledsPerSide);
    void tick();
    void turnOn(int t);
    void turnOff(int t);
    int level;
    LampMode mode;
    boolean orientedUpwards;
  private:
    int _numSides;
    int _ledsPerSide;
    int _updateFrequency = 9;
    unsigned long _lastUpdate;
    int startLevel;
    int endLevel;
    unsigned long startTime;
    unsigned long endTime;
    unsigned long _now;
    CRGB * leds;

    void mapLEDs(int i, int h, int s, int v);

};

#endif
