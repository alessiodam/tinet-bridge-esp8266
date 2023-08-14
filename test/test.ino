#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>

#define BUILTIN_LED 2 // NodeMCU

const char* ssid = "wifissid";
const char* password = "wifipass";
const char* serverAddress = "tinethub.tkbstudios.com";
const int serverPort = 2052;

bool bridge_connected = false;
WiFiClient tcpwificlient;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    blinkLED(1000, 1000);
    Serial.write("WIFI_CONNECTING");
  }

  Serial.write("WIFI_CONNECTED");

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  handleSerialTask();
  handleTcpTask();
}

void connectToTCP() {
  Serial.write("TCP_CONNECTING");
  if (tcpwificlient.connect(serverAddress, serverPort)) {
    Serial.write("TCP_CONNECTED");
    bridge_connected = true;
  } else {
    Serial.write("TCP_CONNECT_ERROR");
    bridge_connected = false;
  }
}

void handleTcpTask() {
  if (tcpwificlient.connected() && Serial.availableForWrite()) {
    while (tcpwificlient.available()) {
      char c = tcpwificlient.read();
      Serial.write(c);
      digitalWrite(BUILTIN_LED, LOW);
      delay(100);
      digitalWrite(BUILTIN_LED, HIGH);
    }
  } else if (!tcpwificlient.connected() && bridge_connected) {
    tcpwificlient.stop();
    Serial.write("TCP_DISCONNECTED");
    bridge_connected = false;
    blinkLED(50, 50);
  }
}

void handleSerialTask() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (tcpwificlient.connected()) {
      tcpwificlient.write(command.c_str());
    }
  }
}

void blinkLED(unsigned long onTime, unsigned long offTime) {
  digitalWrite(BUILTIN_LED, LOW);
  delay(onTime);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(offTime);
}
