#include <ArduinoOTA.h>
#include <WiFiNINA.h>

#include "arduino_secrets.h">
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

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
  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    Serial.print(".");
    delay(1000);
  }
  printWiFiStatus();

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println();
  delay(1000);
  updateStateMachine();
}

void updateStateMachine() {
  switch(currentState) {
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
