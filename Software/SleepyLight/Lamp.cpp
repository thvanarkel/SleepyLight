#include "Arduino.h"
#include "Lamp.h"



Lamp::Lamp(int pin, int numSides, int ledsPerSide)
{
  _pin = pin;
  _numSides = numSides;
  _ledsPerSide = ledsPerSide;
  strip.updateType(NEO_GRB + NEO_KHZ800);
  strip.updateLength((_numSides * _ledsPerSide));
  strip.setPin(_pin);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void Lamp::turnOn()
{
  for (int i = 0; i < _ledsPerSide; i++) {
    for (int j = 0; j < _numSides; j++) {
      int index = (j % 2) ? i : ((_ledsPerSide - 1) - i);
      strip.setPixelColor((j * _ledsPerSide) + index, strip.ColorHSV(6000, 200, 255));
    }
    strip.show();
    delay(150);
  }
}
