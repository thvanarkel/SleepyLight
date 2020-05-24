#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include "Lamp.h"
#include <MQTT.h>
//#include <SD.h>
#include <ArduinoSound.h>

#include <Wire.h>
#include "ds3231.h"
#include "RTClib.h"

#include "arduino_secrets.h">
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

//#include <FastLED.h>

#define LED_TYPE WS2812
#define DATA_PIN 6
#define NUM_LEDS 50

bool turnedOn = false;

WiFiClient net;
MQTTClient client;

RTC_DS3231 rtc;

Lamp lamp(5, 10);

unsigned long lastUpdate;

#define BUFF_MAX 256

uint8_t sleep_period = 1;
unsigned long prev = 5000, interval = 10000;

#define SHUTDOWN_PIN 5

// filename of wave file to play
const char filename[] = "orbit.wav";

//SDWaveFile waveFile;

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
//
  lamp.orientedUpwards = true;
//
//  // Initialise shutdown pin speaker
//  pinMode(SHUTDOWN_PIN, OUTPUT);
//  digitalWrite(SHUTDOWN_PIN, LOW);
////
//  Wire.begin();
//  if (!rtc.begin()) {
//    Serial.println("Couldn't find RTC");
//  }
//
//  if (rtc.lostPower()) {
//    Serial.println("RTC lost power, let's set the time!");
//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  }
//  nextAlarm();
//
//  if (!SD.begin()) {
//    client.publish("/error", "SD initialization failed");
////    return;
//  }
//  waveFile = SDWaveFile(filename);
//  if (!waveFile) {
//    client.publish("/error", "wave file invalid");
//  }
//  AudioOutI2S.volume(60);
//  if (!AudioOutI2S.canPlay(waveFile)) {
//    client.publish("/error", "unable to play wave file using I2S!");
////    while (1); // do nothing
//  }
//
//
//
  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    Serial.print(".");
    delay(1000);
  }
  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);
  connect();

  ArduinoOTA.begin(WiFi.localIP(), "Arduino", "password", InternalStorage);

  programUploaded();

}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect("SleepyLight", "4930afd9", "a7f2cc0b2ba3de3f")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");

  client.subscribe("/turnedOn");
}

void loop() {
  ArduinoOTA.poll();
  lamp.tick();
  if (!AudioOutI2S.isPlaying()) {
    // playback has stopped
    digitalWrite(SHUTDOWN_PIN, LOW);
    client.loop(); // Doing this during I2S outputting sound willl drop connection
  }
//  if (millis() - lastUpdate > 1000) {
//    client.publish("/ledLevel", String(lamp.level));
//    lastUpdate = millis();
//  }
//
//  char buff[BUFF_MAX];
  unsigned long n = millis();
  if ((n - prev > interval)) {
//    DateTime now = rtc.now();
//
//      String time = String(now.day(), DEC) + "." + String(now.month(), DEC) + "." + String(now.year(), DEC) + ": " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
//     // snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", now.year, now.month, now.mday, now.hour, now.min, now.sec);
//     client.publish("/time", time);
//    if (rtc.alarmFired(1)) {
//        rtc.clearAlarm(1);
//        client.publish("/event/alarm", "true");
//
//        if (turnedOn) {
//          lamp.turnOff(8000);
//        } else {
//          lamp.turnOn(2000);
//        }
//        turnedOn = !turnedOn;
//       // digitalWrite(SHUTDOWN_PIN, HIGH);
//       // AudioOutI2S.play(waveFile);
//       nextAlarm();
//        // clear a2 alarm flag and let INT go into hi-z
//        // DS3231_clear_a2f();
//    }
////
    prev = n;
  }

//  updateStateMachine();
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

void programUploaded() {
  // lamp.turnOn(3000);
  // turnedOn = true;
//  client.publish("/test", "test");
}

void nextAlarm()
{
    DateTime now = rtc.now();
    DateTime future(now + TimeSpan(0, 0, 1, 0));

    rtc.setAlarm1(future, DS3231_A1_Minute);
}

void messageReceived(String &topic, String &payload) {
  Serial.println(topic);
  Serial.println(payload);
  if (topic.equals("/turnedOn")) {
    if (payload.equals("true")) {
      lamp.turnOn(3000);
    } else if (payload.equals("false")) {
      lamp.turnOff(3000);
    }
  }
}
