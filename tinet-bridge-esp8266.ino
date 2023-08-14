#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifissidhere"        // temporary, will be on calc soon
#define WIFI_PASSWORD "wifipasshere"    // temporary, will be on calc soon
#define MAX_WIFI_CONNECT_ATTEMPTS 10
#define ENABLE_WEB_SERVER true

#define BUILTIN_LED 2  // NodeMCU and ESP8266

const char* serverAddress = "tinethub.tkbstudios.com";
const int serverPort = 2052;

bool tcp_connected = false;
bool serial_connected = false;
bool toggle_LED = false;

WiFiClient tcpwificlient;
// WiFiClient httpwificlient;
// HTTPClient goodhttpclient;
ESP8266WebServer webserver(80);

unsigned long ledChangeTime = 0;
const unsigned long ledChangeInterval = 100;
unsigned long ledBlinkInterval = 50;

unsigned long transferredPackets = 0;

String calcID = "";
String username = "";


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  Serial.begin(9600);
  
  delay(500);
  Serial.write("ESP8266");
  delay(1000);
  connectWiFi(WIFI_SSID, WIFI_PASSWORD);

  if (ENABLE_WEB_SERVER == true) {
    webserver.on("/", HTTP_GET, handleRoot);
    // webserver.on("/restart", HTTP_GET, handleRestart);
    webserver.begin();
  }

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  handleSerialTask();
  handleTcpTask();
  toggleLED();

  webserver.handleClient();
}

void handleRoot() {
  String html = "<html><head>";
  html += "<style>body{font-family: Arial, sans-serif;}";
  html += ".container{text-align: center; margin-top: 20px;}";
  html += ".body{background-color: black; color: white;}";
  html += ".status{font-weight: bold; color: #007BFF;}";
  html += ".btn{background-color: #007BFF; color: white; padding: 10px 20px;";
  html += "border: none; text-align: center; text-decoration: none; display: inline-block;";
  html += "font-size: 16px; margin: 4px 2px; cursor: pointer;}";
  html += ".counter{margin-top: 20px;}";
  html += ".dashboard{display: flex; flex-direction: column; align-items: center;}";
  html += ".dashboard-section{margin: 10px;}";
  html += ".dashboard-data{font-size: 18px;}";
  html += "</style>";
  html += "</head><body><div class='container'>";
  html += "<div class='dashboard'>";
  html += "<h1>ESP8266 Bridge Dashboard</h1>";
  html += "<div class='dashboard-section'><p class='dashboard-data status'>TCP Connected: " + String(tcp_connected) + "</p></div>";
  html += "<div class='dashboard-section'><p class='dashboard-data status'>Serial Connected: " + String(serial_connected) + "</p></div>";
  html += "<div class='dashboard-section'><p class='dashboard-data'>LED Blink Interval: " + String(ledBlinkInterval) + " ms</p></div>";
  html += "<div class='dashboard-section'><p class='dashboard-data'>Transferred Packets: " + String(transferredPackets) + "</p></div>";
  html += "<div class='dashboard-section'><p class='dashboard-data'>Calc ID: " + calcID + "</p></div>";
  html += "<div class='dashboard-section'><p class='dashboard-data'>Username: " + username + "</p></div>";
  /* ! IN DEV !
  if (!username.isEmpty()) {
    String apiUrl = "https://tinet.tkbstudios.com/api/v1/users/user/" + username;
    goodhttpclient.begin(httpwificlient, apiUrl);
    int httpResponseCode = goodhttpclient.GET();

    if (httpResponseCode == 200) {
      String payload = goodhttpclient.getString();
      DynamicJsonDocument jsonDoc(512);
      DeserializationError err = deserializeJson(jsonDoc, payload);
      if (err) {
        Serial.write("BRIDGE_USER_STATS_WEB_UI_JSON_PARSE_ERROR");
        return;
      }
      bool calcConnected = jsonDoc["calc_connected"];
      int userID = jsonDoc["id"];
      unsigned long lastLogin = jsonDoc["last_login"];
      float mbUsedThisMonth = jsonDoc["mb_used_this_month"];
      String plan = jsonDoc["plan"];
      int requestsThisMonth = jsonDoc["requests_this_month"];
      int totalRequests = jsonDoc["total_requests"];
      html += "<div class='dashboard-section'><p class='dashboard-data status'>TCP Connected: " + String(tcp_connected) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data status'>Serial Connected: " + String(serial_connected) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>LED Blink Interval: " + String(ledBlinkInterval) + " ms</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Transferred Packets: " + String(transferredPackets) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Calc ID: " + calcID + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Username: " + username + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Calc Connected: " + String(calcConnected) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>User ID: " + String(userID) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Last Login: " + String(lastLogin) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>MB Used This Month: " + String(mbUsedThisMonth, 3) + " MB</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Plan: " + plan + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Requests This Month: " + String(requestsThisMonth) + "</p></div>";
      html += "<div class='dashboard-section'><p class='dashboard-data'>Total Requests: " + String(totalRequests) + "</p></div>";
    }
    goodhttpclient.end();
  }
  */
  html += "</div>";
  html += "</div></body></html>";
  webserver.send(200, "text/html", html);
}

void handleRestart() {
  webserver.send(200, "text/plain", "Restarting ESP...");
  delay(1000);
  ESP.restart();
}

void connectToTCP() {
  if (tcpwificlient.connect(serverAddress, serverPort)) {
    tcp_connected = true;
    Serial.write("bridgeConnected");
  } else {
    Serial.write("TCP_CONNECT_ERROR");
    toggle_LED = true;
    tcp_connected = false;
    delay(50);
  }
}

void handleTcpTask() {
  if (!tcp_connected) {
    if (!tcpwificlient.connected()) {
      connectToTCP();
    }
    return;
  }

  if (tcpwificlient.connected()) {
    while (tcpwificlient.available() && Serial.availableForWrite()) {
      char c = tcpwificlient.read();
      if (c != '\0') {
        Serial.write(c);
        toggle_LED = true;
        transferredPackets++;
      }
    }
    tcpwificlient.flush();
  } else {
    tcp_connected = false;
    tcpwificlient.stop();
    Serial.write("TCP_DISCONNECTED");
    blinkLED(50, 50);
  }
}

void handleSerialTask() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (tcp_connected) {
      if (command.startsWith("LOGIN:")) {
        int calcIDStart = 6;
        int calcIDEnd = command.indexOf(":", calcIDStart);
        int usernameStart = calcIDEnd + 1;
        int usernameEnd = command.indexOf(":", usernameStart);
        
        if (calcIDEnd >= 0 && usernameStart >= 0 && usernameEnd >= 0) {
          calcID = command.substring(calcIDStart, calcIDEnd);
          username = command.substring(usernameStart, usernameEnd);
        }
        tcpwificlient.write(command.c_str());
        if (!command.isEmpty()) {
          toggle_LED = true;
          transferredPackets++;
        }
        Serial.flush();
      } else {
        tcpwificlient.write(command.c_str());
        if (!command.isEmpty()) {
          toggle_LED = true;
          transferredPackets++;
        }
        Serial.flush();
      }
    }
  }
}

void toggleLED() {
  if (toggle_LED == true) {
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    delay(ledBlinkInterval);
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    toggle_LED = false;
  }
}

void blinkLED(unsigned long onTime, unsigned long offTime) {
  digitalWrite(BUILTIN_LED, LOW);
  delay(onTime);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(offTime);
}

void connectWiFi(String ssidtoconnect, String passwordtouseforconnect) {
  WiFi.begin(ssidtoconnect, passwordtouseforconnect);
  int wifi_connect_attempts = 0;
  Serial.write("WIFI_CONNECTING");
  while (WiFi.status() != WL_CONNECTED) {
    if (wifi_connect_attempts >= MAX_WIFI_CONNECT_ATTEMPTS)
    {
      Serial.write("WIFI_CONNECT_FAILED");
      break;
    }
    blinkLED(1000, 1000);
    wifi_connect_attempts += 1;
  }
  Serial.write("WIFI_CONNECTED");
}

