#include "Arduino.h"
#include "Lamp.h"

Lamp::Lamp(int numSides, int ledsPerSide)
{
  _numSides = numSides;
  _ledsPerSide = ledsPerSide;

  int nLeds = _numSides * _ledsPerSide;

  leds = new CRGB[nLeds];
  LEDS.addLeds<WS2812,6,GRB>(leds,(_numSides * _ledsPerSide));
  LEDS.setBrightness(255);
}

void Lamp::tick() {
  if (millis() - _lastUpdate > _updateFrequency) {
    for(int i = 0; i < (_numSides * _ledsPerSide); i++) {
		  leds[i] = CHSV(20, quadwave8(pos), quadwave8(pos));
      leds[i] = CHSV(20, 255 - pos, 255);
	  }
    FastLED.show();
    pos++;
    if (pos % 255 == 0) {
      hue += 5;
    }
    _lastUpdate = millis();
  }

}

void Lamp::turnOn()
{
  // brightness = 255;
}

void Lamp::turnOff()
{
  // brightness = 0;
}
