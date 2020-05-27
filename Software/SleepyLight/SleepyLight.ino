#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include "Lamp.h"
#include <MQTT.h>
#include <SD.h>
#include <ArduinoSound.h>
#include <Arduino_LSM6DS3.h>
#include "SensorFusion.h"

#include <Wire.h>
#include "ds3231.h"
#include "RTClib.h"

#include "arduino_secrets.h">
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

#define LED_TYPE WS2812
#define DATA_PIN 6
#define NUM_LEDS 50

bool turnedOn = false;

WiFiClient net;
MQTTClient client;

RTC_DS3231 rtc;

char bedtimeDays[8] = "0111110";
char wakeupDays[8] = "0111110";
String bedtimeAlarm;
String wakeupAlarm;

Lamp lamp(5, 10);
unsigned long lastUpdate;

uint8_t sleep_period = 1;
unsigned long prev = 5000, interval = 1000;


// State parameters

int slumberIntensity = 75;
int slumberDecay = 10000;

int awakePeriod = 2; // in minutes




#define SHUTDOWN_PIN 5

struct Sound {
  char filename[14];
  int duration;
};

static Sound sounds[8] = {{"birdsong.wav", 61},
  {"bright.wav", 61},
  {"droplets.wav", 61},
  {"early.wav", 54},
  {"helios.wav", 64},
  {"orbit.wav", 64},
  {"spring.wav", 70},
  {"sunny.wav", 60}
};

int currentSound = 5;
unsigned long startedPlaying;

SDWaveFile waveFile;

Orientation orientation;
boolean didTurn;
int nShakes;

enum Movement {
  PICKUP = 0,
  SHAKE = 1,
  TURN = 2
};

enum State {
  LIGHT_IDLE,
  UNWINDING,
  SLEEPING,
  AWAKENING,
  ALARMED,
  SLUMBERING
};

State currentState = LIGHT_IDLE;
State lastState;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("connecting");
  //
  lamp.orientation = orientation;

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

  // Initialise RTC
  Wire.begin();
  if (!rtc.begin()) {
    client.publish("/error", "Couldn't find RTC");
  }
  //
  if (rtc.lostPower()) {
    client.publish("/error", "RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialise SD card reader and I2S speaker
  if (!SD.begin()) {
    client.publish("/error", "SD initialization failed");
    //    return;
  }
  waveFile = SDWaveFile(sounds[currentSound].filename);
  if (!waveFile) {
    client.publish("/error", "wave file invalid");
  }
  AudioOutI2S.volume(60);
  if (!AudioOutI2S.canPlay(waveFile)) {
    client.publish("/error", "unable to play wave file using I2S!");
    //    while (1); // do nothing
  }

  // Initialise shutdown pin speaker
  pinMode(SHUTDOWN_PIN, OUTPUT);
  digitalWrite(SHUTDOWN_PIN, LOW);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
  }

  ArduinoOTA.begin(WiFi.localIP(), "Arduino", "password", InternalStorage);


  updateOrientation();
  detectTurn(orientation);

  client.publish("/state", String(currentState));
  lastState = currentState;
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect("SleepyLight", USERNAME, PASSWORD)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");

  client.subscribe("/turnedOn");
  client.subscribe("/wakeup/alarm");
  client.subscribe("/wakeup/days");
  client.subscribe("/bedtime/alarm");
  client.subscribe("/bedtime/days");
  //  client.subscribe("/ledLevel");
}

boolean disconnected = false;

void loop() {
  ArduinoOTA.poll();
  lamp.tick();
  //  Serial.println(lamp.level);

  if (!client.connected()) {
    connect();
  }
  client.loop();

  if (!AudioOutI2S.isPlaying()) {
    // playback has stopped
    digitalWrite(SHUTDOWN_PIN, LOW);
    // Doing this during I2S outputting sound willl drop connection
  }

  if (currentState != lastState) {
    client.publish("/state", String(currentState));
    lastState = currentState;
  }

  if (millis() - lastUpdate > 1000) {
    client.publish("/ledLevel", String(lamp.level));

    //    client.publish("/orientation", String(orientation));
    lastUpdate = millis();
  }

  updateOrientation();
  detectMovement();


  //  //
  //  //  char buff[BUFF_MAX];
  unsigned long n = millis();
  if ((n - prev > interval)) {
    DateTime now = rtc.now();
    publishDate("/time", now);
    //      if (rtc.alarmFired(1)) {
    //        rtc.clearAlarm(1);
    //        client.publish("/event/alarm", "true");
    //  //      //
    //  //      //      if (turnedOn) {
    //  //      //        lamp.turnOff(8000);
    //  //      //      } else {
    //  //      //        lamp.turnOn(2000);
    //  //      //      }
    //  //      //      turnedOn = !turnedOn;
    //        digitalWrite(SHUTDOWN_PIN, HIGH);
    //        AudioOutI2S.play(waveFile);
    //  //      //      //       nextAlarm();
    //  //      //      //        // clear a2 alarm flag and let INT go into hi-z
    //  //      //      //        // DS3231_clear_a2f();
    //      }
    prev = n;
  }

  updateStateMachine();

  delay(10);
}

void updateStateMachine() {
  Serial.println(currentState);
  switch (currentState) {
    case LIGHT_IDLE:

      // Check if the lamp is turned
      if (detectTurn(orientation)) {
        lamp.orientation = orientation;
        lamp.turnOn(2000);
        didTurn = true;
      }
      // If the lamp is turned and fully on, transition to next state
      if (didTurn) {
        if (lamp.level >= 1023) {
          didTurn = false;
          currentState = UNWINDING;
          lamp.turnOff(10000);
        }
      }
      // Check if the wake-up alarm has fired
      if (rtc.alarmFired(1)) {
        rtc.clearAlarm(1);
        DateTime now = rtc.now();
        DateTime future(now + TimeSpan(0, 0, 2, 0)); // TODO: fix the times
        rtc.setAlarm1(future, DS3231_A1_Hour);
        lamp.turnOn(2 * 60 * 1000); // TODO: fix the times
        currentState = AWAKENING;
      }

      if (!lamp.inAnimation() && nShakes > 3) {
        lamp.setLevel(slumberIntensity, 1500);
        nShakes = 0;
        currentState = SLUMBERING;
      }

      break;

    case UNWINDING:
      if (lamp.level <= 0) {
        currentState = LIGHT_IDLE;
      }
      break;

    case SLEEPING:
      break;

    case AWAKENING:
      if (rtc.alarmFired(1)) {
        rtc.clearAlarm(1);
        playSound(true);
        nShakes = 0;
        // TODO: set the alarm for the next day!
        // TODO: check for calendar days
        currentState = ALARMED;
      }
      break;

    case ALARMED:
      if (!AudioOutI2S.isPlaying()) {
        // playback has stopped
        playSound(true);
      }
      if (detectTurn(orientation)) {
        stopSound();
        lamp.orientation = orientation;
        lamp.turnOff(2000);
        nextAlarm(true);
        currentState = LIGHT_IDLE;
      }
      if (nShakes > 3) {
        stopSound();
        nShakes = 0;
        DateTime now = rtc.now();
        DateTime future(now + TimeSpan(0, 0, 1, 0)); // TODO: fix the times
        rtc.setAlarm1(future, DS3231_A1_Hour);
        currentState = AWAKENING;
      }
      break;

    case SLUMBERING:
      if (!lamp.inAnimation()) {
        if (lamp.level >= slumberIntensity) {
          lamp.turnOff(slumberDecay);
        } else {
          currentState = LIGHT_IDLE;
        }
      }

      break;

  }
}


String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setAlarm(String str, boolean wakeup) {
  //(year, month, day, hour, minute, second)
  
  int alarmIndex = wakeup ? 1 : 2;
  rtc.clearAlarm(alarmIndex);
  DateTime now = rtc.now();
  DateTime a(now.year(), now.month(), now.day(), getValue(str, ':', 0).toInt(), getValue(str, ':', 1).toInt(), 0);

  if (now < a) {
    // Still do the alarm today
    char theDay = wakeup ? wakeupDays[now.dayOfTheWeek()] : bedtimeDays[now.dayOfTheWeek()];
    if (theDay == '1') {
      DateTime future = a - TimeSpan(0, 0, 2, 0);
      if (wakeup) {
        rtc.setAlarm1(future, DS3231_A1_Hour);
        publishDate("/wakeup/alarm/time", future);
      } else {
        rtc.setAlarm2(future, DS3231_A2_Hour);
        publishDate("/bedtime/alarm/time", future);
      }
    } else {
      nextAlarm(wakeup);
    }
  } else {
    // Schedule next alarm
    nextAlarm(wakeup);
  }


  //  rtc.setAlarm1(future, DS3231_A1_Hour);

  //  char buffer[] = "hh:mm:ss";
  //  String msg = future.toString(buffer);
  //  client.publish("/light-alarm", msg);
}


void nextAlarm(boolean wakeup)
{
  //  DateTime future(now + TimeSpan(0, 0, 1, 0));
  //
  //  rtc.setAlarm1(future, DS3231_A1_Minute);
  DateTime now = rtc.now();
  String str = wakeup ? wakeupAlarm : bedtimeAlarm;
  DateTime a(now.year(), now.month(), now.day(), getValue(str, ':', 0).toInt(), getValue(str, ':', 1).toInt(), 0);
  boolean foundNext = false;
  int i = 1;
  while(!foundNext) {
    int index = now.dayOfTheWeek() + i > 6 ? now.dayOfTheWeek() + i - 7 : now.dayOfTheWeek() + i;
    char d = wakeup ? wakeupDays[index] : bedtimeDays[index];  
    if (d != '1') {
      i++;
    } else {
      foundNext = true;
      DateTime n = a + TimeSpan(i, 0, 0, 0);
      DateTime future = n - TimeSpan(0, 0, awakePeriod, 0);
      if (wakeup) {
        rtc.setAlarm1(future, DS3231_A1_Hour);
        publishDate("/wakeup/alarm/time", future);
      } else {
        rtc.setAlarm2(future, DS3231_A2_Hour);
        publishDate("/bedtime/alarm/time", future);
      }
    }
    if (i > 7) {
      if (wakeup) {
        int alarmIndex = wakeup ? 1 : 2;
        rtc.clearAlarm(alarmIndex);
      }
      break; // No dates selected
    }
  }
}

void publishDate(String topic, DateTime date) {
  String time = String(date.dayOfTheWeek(), DEC) + " " + String(date.day(), DEC) + "." + String(date.month(), DEC) + "." + String(date.year(), DEC) + ": " + String(date.hour(), DEC) + ":" + String(date.minute(), DEC) + ":" + String(date.second(), DEC);
  //    //         // snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", now.year, now.month, now.mday, now.hour, now.min, now.sec);
  client.publish(topic, time);
}

void messageReceived(String &topic, String &payload) {
  Serial.println(topic);
  Serial.println(payload);
  if (payload.equals("true") && lamp.level <= 0) {
    lamp.turnOn(2000);
  } else if (payload.equals("false") && lamp.level >= 1023) {
    lamp.turnOff(2000);
  } else if (topic.equals("/wakeup/alarm")) {
    wakeupAlarm = payload;
    setAlarm(payload, true);
  } else if (topic.equals("/wakeup/days")) {
    payload.toCharArray(wakeupDays, 8);
  } else if (topic.equals("/bedtime/alarm")) {
    bedtimeAlarm = payload;
    setAlarm(payload, false);
  } else if (topic.equals("/bedtime/days")) {
    payload.toCharArray(bedtimeDays, 8);
  }
}

void setSound(int index) {
  if (currentSound != index) {
    currentSound = index;
    waveFile = SDWaveFile(sounds[currentSound].filename);
    if (!waveFile) {
      client.publish("/error", "wave file invalid");
    }
  }
}

void playSound(boolean looping) {
  digitalWrite(SHUTDOWN_PIN, HIGH);
  if (looping) {
    AudioOutI2S.loop(waveFile);
  } else {
    AudioOutI2S.play(waveFile);
  }

  startedPlaying = millis();
}

void stopSound() {
  digitalWrite(SHUTDOWN_PIN, LOW);
  AudioOutI2S.stop();
}
