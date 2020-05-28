#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include "Lamp.h"
#include <MQTT.h>
#include <SD.h>
#include <SDConfigCommand.h>
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

//char bedtimeDays[8] = "0111110";
//char wakeupDays[8] = "0111110";
//String bedtimeAlarm;
//String wakeupAlarm;

Lamp lamp(5, 10);
unsigned long lastUpdate;

uint8_t sleep_period = 1;
unsigned long prev = 5000, interval = 1000;


struct Config {
  String key;
  String value;
};


#define NUM_CONFIGS 9

Config defaultConfig[NUM_CONFIGS] = {
  {"bedtimedays", "000000"},
  {"wakeupdays", "000000"},
  {"wakeupalarm", "8:00:00"},
  {"bedtimealarm", "23:00:00"},
  {"awakeningtime", "1"},
  {"unwinddecay", "30"},
  {"slumberdecay", "30"},
  {"snooze", "8"},
  {"currentsound", "5"}
};

Config configuration[NUM_CONFIGS] = {
  {"bedtimedays", ""},
  {"wakeupdays", ""},
  {"wakeupalarm", ""},
  {"bedtimealarm", ""},
  {"awakeningtime", ""},
  {"unwinddecay", ""},
  {"slumberdecay", ""},
  {"snooze", ""},
  {"currentsound", ""}
};


// State parameters

int slumberIntensity = 65;
//int slumberDecay = 30 * 1000;
int slumberShake = 2500;

//int awakeningPeriod = 1; // in minutes

//int unwindDecay = 30;

//int snoozePeriod = 8; // in minutes


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

SDConfigCommand sdcc;
SDWaveFile waveFile;

Orientation orientation = NONE;
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

  // check config
  loadConfig();

  currentSound = getConfig("currentsound").toInt();
  Serial.println(currentSound);
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
  lamp.orientation = orientation;

  client.publish("/state", String(currentState));
  lastState = currentState;
}


int cIndex = 0;

void loadConfig() {
  Serial.println("Creating example.txt...");

  File myFile;

  if (!SD.exists("setting.cfg")) {
    Serial.println("file does not exist");
    myFile = SD.open("setting.cfg", FILE_WRITE);
    writeDefaultConfig(myFile);
    myFile.close();
  } else {
    client.publish("/config", "file does exist");
  }
  // Load the configuration
  while ( !(sdcc.set("setting.cfg", 4, processCmd)) ) {}

  sdcc.readConfig();
  
}

void writeDefaultConfig(File file) {
  for (int i = 0; i < NUM_CONFIGS; i++) {
    Config c = defaultConfig[i];
    file.print(c.key);
    file.print("=");
    file.println(c.value);
  }
}

void setConfig(String key, String value) {
  for (int i = 0; i < NUM_CONFIGS; i++) {
    Config c = configuration[i];
    if (c.key.equals(key)) {
      c.value = value;
      sdcc.writeConfig(key, value);
    }
  }
}

String getConfig(String key) {
  Serial.println(key);
  for (int i = 0; i < NUM_CONFIGS; i++) {
    Config c = configuration[i];
    if (c.key.equals(key)) {
      return c.value;
    }
  }
}

void processCmd() {
  // This function will run every time there is a command
  // Maybe check the key?
  String k = sdcc.getCmdS();
  String v = sdcc.getValueS();

  if (cIndex < NUM_CONFIGS) {
    configuration[cIndex].key = k;
    configuration[cIndex].value = v;
    cIndex++;
  }
  
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
  client.subscribe("/wakeup/#");
  client.subscribe("/bedtime/#");
  client.subscribe("/awakeTime");
  client.subscribe("/unwindTime");
  client.subscribe("/slumberTime");
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
  lamp.orientation = orientation;

  unsigned long n = millis();
  if ((n - prev > interval)) {
    DateTime now = rtc.now();
    publishDate("/time", now);
    prev = n;
  }

  updateStateMachine();

  delay(10);
}

void updateStateMachine() {
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
          lamp.turnOff(getConfig("unwinddecay").toInt() * 60 * 1000);
        } // TODO: Turn this into a function
      }
      // Check if the wake-up alarm has fired
      if (rtc.alarmFired(1)) {
        rtc.clearAlarm(1);
        DateTime now = rtc.now();
        DateTime future(now + TimeSpan(0, 0, getConfig("awakeningtime").toInt(), 0));
        rtc.setAlarm1(future, DS3231_A1_Hour);
        lamp.turnOn(getConfig("awakeningtime").toInt() * 60 * 1000);
        currentState = AWAKENING;
      }

      if (!lamp.inAnimation() && nShakes > 3) {
        lamp.setLevel(slumberIntensity, 1500);
        nShakes = 0;
        currentState = SLUMBERING;
      }

      break;

    case UNWINDING:
      if (!lamp.inAnimation()) {
        currentState = LIGHT_IDLE;
      }
      if (nShakes > 3) {
        nShakes = 0;
        lamp.changeTime((-60 * 1000));
        client.publish("/endtime", String(lamp.endTime));
      }
      break;

    case SLEEPING:
      break;

    case AWAKENING:
      if (rtc.alarmFired(1)) {
        rtc.clearAlarm(1);
        playSound();
        nShakes = 0;
        // TODO: set the alarm for the next day!
        // TODO: check for calendar days
        currentState = ALARMED;
      }
      break;

    case ALARMED:
      if (!AudioOutI2S.isPlaying()) {
        // playback has stopped
        playSound();
      } else {
        loopSound();
      }
      if (nShakes > 3) {
        stopSound();
        nShakes = 0;
        DateTime now = rtc.now();
        DateTime future(now + TimeSpan(0, 0, getConfig("snooze").toInt(), 0)); // TODO: fix the times
        rtc.setAlarm1(future, DS3231_A1_Hour);
        currentState = AWAKENING;
        break;
      }
      if (detectTurn(orientation)) {
        stopSound();
        lamp.orientation = orientation;
        lamp.turnOff(2000);
        nextAlarm(true);
        currentState = LIGHT_IDLE;
        break;
      }
      break;

    case SLUMBERING:
      if (!lamp.inAnimation()) {
        if (lamp.level >= slumberIntensity) {
          lamp.turnOff(getConfig("slumberdecay").toInt() * 1000);
        } else {
          currentState = LIGHT_IDLE;
        }
      }
      if (nShakes > 2) {
        lamp.setLevel(slumberIntensity, 500);
        nShakes = 0;
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
    char theDay;
    if (wakeup) {
      char wd[8];
      getConfig("wakeupdays").toCharArray(wd, 8);
      theDay = wd[now.dayOfTheWeek()];
    } else {
      char bd[8];
      getConfig("bedtimedays").toCharArray(bd, 8);
      theDay = bd[now.dayOfTheWeek()];
    }
    if (theDay == '1') {
      DateTime future = a - TimeSpan(0, 0, getConfig("awakeningtime").toInt(), 0);
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
}


void nextAlarm(boolean wakeup)
{
  //  DateTime future(now + TimeSpan(0, 0, 1, 0));
  //
  //  rtc.setAlarm1(future, DS3231_A1_Minute);
  DateTime now = rtc.now();
  String str = wakeup ? getConfig("wakeupalarm") : getConfig("bedtimealarm");
  DateTime a(now.year(), now.month(), now.day(), getValue(str, ':', 0).toInt(), getValue(str, ':', 1).toInt(), 0);
  boolean foundNext = false;
  int i = 1;
  while (!foundNext) {
    int index = now.dayOfTheWeek() + i > 6 ? now.dayOfTheWeek() + i - 7 : now.dayOfTheWeek() + i;
    char wd[8];
    getConfig("wakeupdays").toCharArray(wd, 8);
    char d = wakeup ? wd[index] : wd[index];
    if (d != '1') {
      i++;
    } else {
      foundNext = true;
      DateTime n = a + TimeSpan(i, 0, 0, 0);
      DateTime future = n - TimeSpan(0, 0, getConfig("awakeningtime").toInt(), 0);
      if (wakeup) {
        rtc.setAlarm1(future, DS3231_A1_Date);
        publishDate("/wakeup/alarm/time", future);
      } else {
        rtc.setAlarm2(future, DS3231_A2_Date);
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
    setConfig("wakeupalarm", payload);
    setAlarm(payload, true);
  } else if (topic.equals("/wakeup/days")) {
    setConfig("wakeupdays", payload);
  } else if (topic.equals("/bedtime/alarm")) {
    setConfig("bedtimealarm", payload);
    setAlarm(payload, false);
  } else if (topic.equals("/bedtime/days")) {
    setConfig("bedtimedays", payload);
  } else if (topic.equals("/bedtime/reminder")) {

  } else if (topic.equals("/awakeTime")) {
    setConfig("awakeningtime", payload);
  } else if (topic.equals("/unwindTime")) {
    setConfig("unwinddecay", payload);
  } else if (topic.equals("/slumberTime")) {
    setConfig("slumberdecay", payload);
  }
}

void setSound(int index) {
  currentSound = getConfig("currentsound").toInt();
  if (currentSound != index) {
    currentSound = index;
    waveFile = SDWaveFile(sounds[currentSound].filename);
    if (!waveFile) {
      client.publish("/error", "wave file invalid");
    }
  }
}

void playSound() {
  digitalWrite(SHUTDOWN_PIN, HIGH);
  AudioOutI2S.play(waveFile);
  startedPlaying = millis();
}

void loopSound() {
  if (millis() - startedPlaying > ((sounds[currentSound].duration - 8) * 1000)) {
    AudioOutI2S.stop();
  }
}

void stopSound() {
  digitalWrite(SHUTDOWN_PIN, LOW);
  AudioOutI2S.stop();
}
