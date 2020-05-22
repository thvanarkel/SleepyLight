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
  FastLED.clear();
  level = 0;
}

void Lamp::tick() {
  if (millis() - _lastUpdate > _updateFrequency) {
    _now = millis();
    level = map(_now, startTime, endTime, startLevel, endLevel);
    level = constrain(level, 0, 1023);
    for (int i = 0; i < _ledsPerSide; i++) {
      // mapLEDs(i, 20, 150, (beatsin8(5, 0, 255, 0, i * 5)));
      int v = 255 - (255 * exp(-(0.0008 * (i + 1)) * level)); // - i * ((sin8(level >> 2))/2);
      mapLEDs(i, 20, 60 + 140 * exp( -0.005 * (level >> 2)), constrain(v, 0, 255));
    }

    FastLED.show();
    _lastUpdate = millis();
  }
}

void Lamp::turnOn(int t)
{
  // brightness = 255;
  startLevel = level;
  endLevel = 1023;

  unsigned long now = millis();
  startTime = now;
  endTime = now + t;
}

void Lamp::turnOff(int t)
{
  // brightness = 0;
  startLevel = level;
  endLevel = 0;

  unsigned long now = millis();
  startTime = now;
  endTime = now + t;
}

void Lamp::mapLEDs(int i, int h, int s, int v)
{
  for (int n = 0; n < _numSides; n++) {
    int index = n % 2 ? (n * _ledsPerSide) + i : (n * _ledsPerSide) + ((_ledsPerSide - 1) - i);
    if (orientedUpwards == false) {
      index = n % 2 ? (n * _ledsPerSide) + ((_ledsPerSide - 1) - i) : (n * _ledsPerSide) + i;
    }
    leds[index] = CHSV(h, s, constrain(v, 0, 255));
  }
}
