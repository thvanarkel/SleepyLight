#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include "Lamp.h"
#include <MQTT.h>

#include "arduino_secrets.h">
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

#include <FastLED.h>

#define LED_TYPE WS2812
#define DATA_PIN 6
#define NUM_LEDS 50

WiFiClient net;
MQTTClient client;

Lamp lamp(5, 10);

unsigned long rainbowUpdate = 0;
long hue = 0;

enum State {
  LIGHT_IDLE,
  UNWINDING,
  SLEEPING,
  AWAKENING
};

State currentState = LIGHT_IDLE;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("connecting");

  programUploaded();

  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    Serial.print(".");
    delay(1000);
  }
  client.begin("broker.shiftr.io", net);
  connect();

  ArduinoOTA.begin(WiFi.localIP(), "Arduino", "password", InternalStorage);

  printWiFiStatus();

}

void connect(boolean networkReconnect, boolean brokerReconnect) {
  Serial.print("checking wifi...");
  int ticks;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    ticks++;
    delay(1000);
    if (ticks > 5) {
      WiFi.begin(ssid, pass);
      ticks = 0;
    }
  }
  Serial.print("\nconnecting...");
  while (!client.connect("SleepyLight", "4930afd9", "a7f2cc0b2ba3de3f")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
}

void loop() {
  ArduinoOTA.poll();
  lamp.tick();

  // rainbow(10);
  // if (millis() > 20000) {
  //   lamp.turnOff();
  // }

  updateStateMachine();
}

void updateStateMachine() {
  switch (currentState) {
    case LIGHT_IDLE:
      break;

    case UNWINDING:
      break;

    case SLEEPING:
      break;

    case AWAKENING:
      break;
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void rainbow(int wait) {
  // // Hue of first pixel runs 5 complete loops through the color wheel.
  // // Color wheel has a range of 65536 but it's OK if we roll over, so
  // // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  // if (millis() < rainbowUpdate + wait) {
  //   return;
  // }
  // rainbowUpdate = millis();
  //
  // for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
  //   // Offset pixel hue by an amount to make one full revolution of the
  //   // color wheel (range of 65536) along the length of the strip
  //   // (strip.numPixels() steps):
  //   int pixelHue = hue + (i * 65536L / strip.numPixels());
  //   // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
  //   // optionally add saturation and value (brightness) (each 0 to 255).
  //   // Here we're using just the single-argument hue variant. The result
  //   // is passed through strip.gamma32() to provide 'truer' colors
  //   // before assigning to each pixel:
  //   // strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  // }
  // // strip.show(); // Update strip with new contents
  //
  // if (hue < 5 * 65536) {
  //   hue += 256;
  // } else {
  //   hue = 0;
  // }
}

void programUploaded() {
  // for (int i = 0; i < LED_COUNT; i++) {
  //   strip.setPixelColor(i, 200, 255, 150);
  //   strip.show();
  //   delay(250);
  //   strip.setPixelColor(i, 0, 0, 0);
  // }
  lamp.turnOn();
  // lamp.turnOff();
}
