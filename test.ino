#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <TaskScheduler.h>

const char* ssid = "wifissid";
const char* password = "wifipass";
const char* serverAddress = "tinethub.tkbstudios.com";
const int serverPort = 2052;

WiFiClient client;
Scheduler scheduler;

Task tcpTask(100, TASK_FOREVER, &handleTcpTask);
Task serialTask(100, TASK_FOREVER, &handleSerialTask);

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  scheduler.init();
  scheduler.addTask(tcpTask);
  scheduler.addTask(serialTask);
  tcpTask.enable();
  serialTask.enable();
}

void loop() {
  scheduler.execute();
}

void handleTcpTask() {
  if (client.connected() && Serial.availableForWrite()) {
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  } else if (!client.connected()) {
    client.stop();
    Serial.println("Disconnected from TCP server");
  }
}

void handleSerialTask() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "CONNECT_TCP" && !client.connected()) {
      Serial.println("Connecting to TCP server...");
      if (client.connect(serverAddress, serverPort)) {
        Serial.println("Connected to TCP server");
      } else {
        Serial.println("Failed to connect to TCP server");
      }
    }
  }
}
