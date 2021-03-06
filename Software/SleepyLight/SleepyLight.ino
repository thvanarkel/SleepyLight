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


#define NUM_CONFIGS 10

Config defaultConfig[NUM_CONFIGS] = {
  {"bedtimedays", "000000"},
  {"wakeupdays", "000000"},
  {"wakeupalarm", "8:00:00"},
  {"bedtimealarm", "23:00:00"},
  {"awakeningtime", "30"},
  {"unwinddecay", "30"},
  {"slumberdecay", "30"},
  {"snooze", "8"},
  {"currentsound", "3"},
  {"bedtimereminder", "30"}
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
  {"currentsound", ""},
  {"bedtimereminder", ""}
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

int currentSound;
unsigned long startedPlaying;

SDConfigCommand sdcc;
SDWaveFile waveFile;

unsigned long reminded;

Orientation orientation;
Orientation lOrientation;
boolean didTurn;
int nShakes;

enum Movement {
  PICKUP = 0,
  SHAKE = 1,
  TURN = 2
};

enum State {
  LIGHT_IDLE,
  REMINDING,
  UNWINDING,
  SLUMBERING,
  AWAKENING,
  ALARMED
};

State currentState = LIGHT_IDLE;
State lastState;

void setup() {
  // put your setup code here, to run once:
  //  Serial.begin(115200);
  //  Serial.print("connecting");
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

  client.publish("/restart", "1");


  // Initialise SD card reader and I2S speaker
  if (!SD.begin()) {
    client.publish("/error", "SD initialization failed");
    //    return;
  }

//  SD.remove("setting.cfg"); // Delete configuration file
  // check config
  loadConfig();

  // Initialise RTC
  Wire.begin();
  if (!rtc.begin()) {
    client.publish("/error", "Couldn't find RTC");
  }
  if (rtc.lostPower()) {
    client.publish("/error", "RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  setAlarm(getConfig("wakeupalarm"), true);
  setAlarm(getConfig("bedtimealarm"), false);

  currentSound = getConfig("currentsound").toInt();
  waveFile = SDWaveFile(sounds[currentSound].filename);
  if (!waveFile) {
    client.publish("/error", "wave file invalid");
  }
  AudioOutI2S.volume(60);
  if (!AudioOutI2S.canPlay(waveFile)) {
    client.publish("/error", "unable to play wave file using I2S!");
  }

  // Initialise shutdown pin speaker
  pinMode(SHUTDOWN_PIN, OUTPUT);
  digitalWrite(SHUTDOWN_PIN, LOW);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
  }

  ArduinoOTA.begin(WiFi.localIP(), "SleepyLight", OTA_PASSWORD, InternalStorage);

  updateOrientation();
  // set the initial orientation
  orientation = getOrientation();
  lamp.orientation = orientation;
  client.publish("/orientation", String(orientation));

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
      configuration[i] = c;
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
  client.subscribe("/sound");
  client.subscribe("/snooze");
  //  client.subscribe("/ledLevel");
}

boolean disconnected = false;

void loop() {
  //  watchdog.clear();
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
  if (orientation != lOrientation) {
    client.publish("/orientation", String(orientation));
    lOrientation = orientation;
  }

  if (millis() - lastUpdate > 1000) {
    client.publish("/ledLevel", String(lamp.level));
    lastUpdate = millis();
  }

  updateOrientation();
  detectMovement();
  lamp.orientation = orientation;

  //  unsigned long n = millis();
  //  if ((n - prev > interval)) {
  //    DateTime now = rtc.now();
  //    publishDate("/time", now);
  //    prev = n;
  //  }

  updateStateMachine();

  delay(15);
}

void updateStateMachine() {
  switch (currentState) {
    case LIGHT_IDLE:

      // Check if the lamp is turned
      if (detectTurn(orientation)) {
        lamp.orientation = orientation;
        lamp.turnOn(2000);
        currentState = UNWINDING;
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
      if (rtc.alarmFired(2)) {
        rtc.clearAlarm(2);
        
        setSound(5);
        playSound();
        reminded = millis();
        currentState = REMINDING;
      }

      if (!lamp.inAnimation() && nShakes > 3) {
        lamp.setLevel(slumberIntensity, 1500);
        nShakes = 0;
        currentState = SLUMBERING;
      }

      break;

    case REMINDING:
      lamp.mode = MOVING;
      breakOffSound(8);
      if (nShakes > 0 || millis() - reminded > 60000) {
        lamp.mode = SOLID;
        stopSound();
        currentSound = getConfig("currentsound").toInt();
        currentState = LIGHT_IDLE;
        nextAlarm(false);
      }
      break;

    case UNWINDING:        
      if (!lamp.inAnimation()) {
        if (lamp.level >= 1023) {
          lamp.turnOff(getConfig("unwinddecay").toInt() * 60 * 1000);
        } else if (lamp.level <= 0) {
          currentState = LIGHT_IDLE;
        }
      }
      if (nShakes > 3) {
        nShakes = 0;
        lamp.changeTime((-60 * 1000));
      }
      break;

    case AWAKENING:
      if (rtc.alarmFired(1)) {
        rtc.clearAlarm(1);
        playSound();
        nShakes = 0;
        currentState = ALARMED;
      }
      break;

    case ALARMED:
      if (!AudioOutI2S.isPlaying()) {
        // playback has stopped
        playSound();
      } else {
        breakOffSound(8);
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
        nShakes = 0;
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
      if (detectTurn(orientation)) {
        lamp.orientation = orientation;
        lamp.turnOn(2000);
        currentState = UNWINDING;
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
      DateTime future;
      if (wakeup) {
        int o = getConfig("awakeningtime").toInt();
        future = a - TimeSpan(0, 0, o, 0);
        rtc.setAlarm1(future, DS3231_A1_Hour);
        publishDate("/wakeup/date", future);
      } else {
        future = a - TimeSpan(0, 0, getConfig("bedtimereminder").toInt(), 0);
        rtc.setAlarm2(future, DS3231_A2_Hour);
        publishDate("/bedtime/date", future);
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
      DateTime future;
      if (wakeup) {
        future = n - TimeSpan(0, 0, getConfig("awakeningtime").toInt(), 0);
        rtc.setAlarm1(future, DS3231_A1_Date);
        publishDate("/wakeup/date", future);
      } else {
        future = n - TimeSpan(0, 0, getConfig("bedtimereminder").toInt(), 0);
        rtc.setAlarm2(future, DS3231_A2_Date);
        publishDate("/bedtime/date", future);
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
  String time = String(date.day(), DEC) + "." + String(date.month(), DEC) + "." + String(date.year(), DEC) + ": " + String(date.hour(), DEC) + ":" + String(date.minute(), DEC) + ":" + String(date.second(), DEC);
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
    setConfig("bedtimereminder", payload);
    setAlarm(getConfig("bedtimealarm"), false);
  } else if (topic.equals("/awakeTime")) {
    setConfig("awakeningtime", payload);
    setAlarm(getConfig("wakeupalarm"), true);
  } else if (topic.equals("/unwindTime")) {
    setConfig("unwinddecay", payload);
  } else if (topic.equals("/slumberTime")) {
    setConfig("slumberdecay", payload);
  } else if (topic.equals("/snooze")) {
    setConfig("snooze", payload);
  } else if (topic.equals("/sound")) {
    setConfig("currentsound", payload);
    setSound(payload.toInt());
  }
}

void setSound(int index) {
  currentSound = index;
  waveFile = SDWaveFile(sounds[currentSound].filename);
  if (!waveFile) {
    client.publish("/error", "wave file invalid");
  }
}

void playSound() {
  digitalWrite(SHUTDOWN_PIN, HIGH);
  AudioOutI2S.play(waveFile);
  startedPlaying = millis();
}

void breakOffSound(int cutoff) {
  if (millis() - startedPlaying > ((sounds[currentSound].duration - cutoff) * 1000)) {
    stopSound();
  }
}

void stopSound() {
  digitalWrite(SHUTDOWN_PIN, LOW);
  AudioOutI2S.stop();
}
