#include "Arduino.h"
#include "Lamp.h"

Lamp::Lamp(int numSides, int ledsPerSide)
{
  _numSides = numSides;
  _ledsPerSide = ledsPerSide;

  int nLeds = _numSides * _ledsPerSide;

  leds = new CRGB[nLeds];
  LEDS.addLeds<WS2812, 6, GRB>(leds, (_numSides * _ledsPerSide));
  FastLED.setBrightness(200);
  FastLED.setTemperature(0xFF9329);
  FastLED.setDither( 0 );
  FastLED.clear();
  level = 0;
}

void Lamp::tick() {
  if (millis() - _lastUpdate > _updateFrequency) {
    _now = millis();
    _now = constrain(_now, startTime, endTime);

    
    
    level = map(_now, startTime, endTime, startLevel, endLevel);
    level = constrain(level, 0, 1023);
//
//    Serial.print(level);
//    Serial.print(": ");
//    Serial.print(_now);
//    Serial.print(" ");
//    Serial.print(startTime);
//    Serial.print(" ");
//    Serial.print(endTime);
//    Serial.print(" ");
//    Serial.print(startLevel);
//    Serial.print(" ");
//    Serial.println(endLevel);
//    
    for (int i = 0; i < _ledsPerSide; i++) {
//      int v = ((level/1023.0) * 255) - (80 * exp(-(0.00099 * (1.25 * i + 1)) * level));
      float v = ((constrain(1.4 * (i + 1) * level, 0, 1023)/1023) * 255);
      float s = (170 - (constrain((2.4 * level), 0, 1023)/1023 * 80));
//      float v = (level/1023.0) * 255;
//      mapLEDs(i, 20, 80 + 140 * exp( -0.005 * level), constrain(v, 0, 255));
      mapLEDs(i, 20, constrain(s, 0, 255), constrain(v, 0, 255));
       
    }

    FastLED.show();
    _lastUpdate = millis();
  }
}

void Lamp::turnOn(int t)
{
  // brightness = 255;
  startLevel = (level + 1);
  endLevel = 1023;
  level += 1;

  unsigned long now = millis();
  startTime = now;
  endTime = now + t;
}

void Lamp::turnOff(int t)
{
  // brightness = 0;
  startLevel = (level - 1);
  endLevel = 0;
  level -= 1;

  unsigned long now = millis();
  startTime = now;
  endTime = now + t;
}

void Lamp::setLevel(int l, int t) {
  startLevel = level;
  endLevel = l;

  unsigned long now = millis();
  startTime = now;
  endTime = now + t;
}

void Lamp::changeTime(int t) {
  if (endTime + t > millis()) {
    endTime += t;
  }
}

boolean Lamp::inAnimation() {
  return level != endLevel ? true : false;
}

void Lamp::mapLEDs(int i, int h, int s, int v)
{
  for (int n = 0; n < _numSides; n++) {
    int index = n % 2 ? (n * _ledsPerSide) + i : (n * _ledsPerSide) + ((_ledsPerSide - 1) - i);
    if (orientation == DOWNWARD) {
      index = n % 2 ? (n * _ledsPerSide) + ((_ledsPerSide - 1) - i) : (n * _ledsPerSide) + i;
    }
    leds[index] = CHSV(20, constrain(s, 0, 255) , constrain(v, 0, 255));
  }
}
