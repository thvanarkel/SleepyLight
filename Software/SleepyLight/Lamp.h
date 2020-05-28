#ifndef Lamp_h
#define Lamp_h

#include "Arduino.h"


#include <FastLED.h>

enum LampMode {
  SOLID,
  MOVING
};

enum Orientation {
  NONE = 0,
  UPWARD = 1,
  DOWNWARD = 2,
  
};

class Lamp {
  public:
    Lamp(int numSides, int ledsPerSide);
    void tick();
    void turnOn(int t);
    void turnOff(int t);
    void setLevel(int l, int t);
    void changeTime(int t);
    boolean inAnimation();
    int level;
    unsigned long endTime;
    LampMode mode = SOLID;
    Orientation orientation = UPWARD;
  private:
    int _numSides;
    int _ledsPerSide;
    int _updateFrequency = 25;
    unsigned long _lastUpdate;
    int startLevel;
    int endLevel;
    unsigned long startTime;
    
    unsigned long _now;
    CRGB * leds;

    void mapLEDs(int i, int h, int s, int v);

};

#endif
