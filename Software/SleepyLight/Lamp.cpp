#include "Arduino.h"
#include "Lamp.h"

Lamp::Lamp(int numSides, int ledsPerSide)
{
  _numSides = numSides;
  _ledsPerSide = ledsPerSide;

  int nLeds = _numSides * _ledsPerSide;

  leds = new CRGB[nLeds];
  LEDS.addLeds<WS2812,6,GRB>(leds,(_numSides * _ledsPerSide));
  LEDS.setBrightness(10);
  FastLED.setTemperature(0xFF9329);
  FastLED.clear();
}

void Lamp::tick() {
  if (millis() - _lastUpdate > _updateFrequency) {
    if (level < toLevel) {
      level += 2;
    } else if (level > toLevel) {
      level -= 2;
    }
    level = constrain(level, 0, 1023);

    for (int i = 0; i < _ledsPerSide; i++) {
      // mapLEDs(i, 20, 150, (beatsin8(5, 0, 255, 0, i * 5)));
      int v = level - i * ((sin8(level >> 2))/10);
      mapLEDs(i, 20, 75, (v >> 2));
    }

    FastLED.show();
    pos++;
    _lastUpdate = millis();
  }
}

void Lamp::turnOn()
{
  // brightness = 255;
  toLevel = 1023;
}

void Lamp::turnOff()
{
  // brightness = 0;
  toLevel = 0;
}

void Lamp::mapLEDs(int i, int h, int s, int v)
{
  for(int n = 0; n < _numSides; n++) {
    int index = n % 2 ? (n * _ledsPerSide) + ((_ledsPerSide - 1) - i) : (n * _ledsPerSide) + i;
    leds[index] = CHSV(h, s, constrain(v, 0, 255));
  }
}
