#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include "Lamp.h"
#include <MQTT.h>
#include <SD.h>
#include <ArduinoSound.h>

#include <Wire.h>
#include "ds3231.h"

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

Lamp lamp(5, 10);

#define BUFF_MAX 256

uint8_t sleep_period = 1;
unsigned long prev = 5000, interval = 5000;

#define SHUTDOWN_PIN 5

// filename of wave file to play
const char filename[] = "orbit.wav";

SDWaveFile waveFile;

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

  // Initialise shutdown pin speaker
  pinMode(SHUTDOWN_PIN, OUTPUT);
  digitalWrite(SHUTDOWN_PIN, LOW);

  Wire.begin();
  DS3231_init(DS3231_INTCN);
  DS3231_clear_a2f();
  set_next_alarm();

  

  if (!SD.begin()) {
    client.publish("/error", "SD initialization failed");
    return;
  }
  waveFile = SDWaveFile(filename);
  if (!waveFile) {
    client.publish("/error", "wave file invalid");
  }
  AudioOutI2S.volume(5);
  if (!AudioOutI2S.canPlay(waveFile)) {
    client.publish("/error", "unable to play wave file using I2S!");
    while (1); // do nothing
  }

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
}

void loop() {
  ArduinoOTA.poll();
  lamp.tick();
  if (!AudioOutI2S.isPlaying()) {
    // playback has stopped
    digitalWrite(SHUTDOWN_PIN, LOW);
    
    client.loop();
  }

  char buff[BUFF_MAX];
  unsigned long now = millis();
  struct ts t;
  if ((now - prev > interval)) {
    DS3231_get(&t);
     snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
     client.publish("/time", buff);
    if (DS3231_triggered_a2()) {
        client.publish("/event/alarm", "true");
        if (turnedOn) {
          lamp.turnOff();
        } else {
          lamp.turnOn();
        }
        turnedOn = !turnedOn;
//        digitalWrite(SHUTDOWN_PIN, HIGH);
//        AudioOutI2S.play(waveFile);
        set_next_alarm();
        // clear a2 alarm flag and let INT go into hi-z
        DS3231_clear_a2f();
    }

    prev = now;
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
  lamp.turnOn();
  turnedOn = true;
//  client.publish("/test", "test");
}

void set_next_alarm(void)
{
    struct ts t;
    unsigned char wakeup_min;

    DS3231_get(&t);

    // calculate the minute when the next alarm will be triggered
    wakeup_min = (t.min / sleep_period + 1) * sleep_period;
    if (wakeup_min > 59) {
        wakeup_min -= 60;
    }

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm
    // A2M2 (minutes) (0 to enable, 1 to disable)
    // A2M3 (hour)    (0 to enable, 1 to disable)
    // A2M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[4] = { 0, 1, 1, 1 };

    // set Alarm2. only the minute is set since we ignore the hour and day component
    DS3231_set_a2(wakeup_min, 0, 0, flags);

    // activate Alarm2
    DS3231_set_creg(DS3231_INTCN | DS3231_A2IE);
}
