#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <EEPROM.h>

const char* ssid = "your-ssid";
const char* password = "your-password";
const int serialBaudRate = 115200;
const int tcpServerPort = 2052;

WiFiServer tcpServer(tcpServerPort);
WiFiClient tcpClient;

AsyncWebServer server(80);

struct Settings {
  char password[64];
  unsigned long transferredPackets;
  float totalMegabytes;
};

Settings settings;

void setup() {
  Serial.begin(serialBaudRate);
  EEPROM.begin(sizeof(Settings));
  loadSettings();
  connectToWiFi();
  setupAsyncServer();
}

void loop() {
  if (tcpClient.connected()) {
    handleSerialToTCP();
    handleTCPToSerial();
  } else {
    handleSerialConnection();
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void setupAsyncServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (strlen(settings.password) == 0) {
      request->send(200, "text/html", "Please set a password <a href='/setpassword'>here</a>.");
    } else {
      String html = "<html><body>";
      html += "<h2>Management Page</h2>";
      html += "<p>Transferred Packets: " + String(settings.transferredPackets) + "</p>";
      html += "<p>Total Megabytes: " + String(settings.totalMegabytes) + "</p>";
      html += "<form action='/setpassword' method='post'>";
      html += "<label>Password: </label>";
      html += "<input type='password' name='password'/>";
      html += "<input type='submit' value='Set'/></form>";
      html += "<form action='/reset' method='post'>";
      html += "<input type='submit' value='Reset to Factory Settings' onclick='return confirm(\"Are you sure?\");'/></form>";
      html += "</body></html>";
      request->send(200, "text/html", html);
    }
  });

  server.on("/setpassword", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<html><body><form action='/savepassword' method='post'><label>Password: </label><input type='password' name='password'/><input type='submit' value='Set'/></form></body></html>");
  });

  server.on("/savepassword", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("password", true)) {
      String newPassword = request->getParam("password", true)->value();
      newPassword.toCharArray(settings.password, sizeof(settings.password));
      saveSettings();
    }
    request->send(200, "text/html", "Password set successfully. <a href='/'>Go to Management Page</a>");
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    resetToFactorySettings();
    request->send(200, "text/html", "Reset to factory settings successful. <a href='/'>Go to Management Page</a>");
  });

  server.begin();
}

void handleSerialConnection() {
  WiFiClient serialClient = tcpServer.available();
  if (serialClient) {
    Serial.println("Serial device connected");
    tcpClient = serialClient;
    tcpClient.setTimeout(5);
  }
}

void handleSerialToTCP() {
  if (tcpClient.available()) {
    String tcpData = tcpClient.readStringUntil('\n');
    settings.transferredPackets++;
    settings.totalMegabytes += tcpData.length() / 1024.0 / 1024.0;  // Assuming 1 character is roughly 1 byte
    Serial.println("Transfer to TINET: " + tcpData);
  }
}

void handleTCPToSerial() {
  if (Serial.available()) {
    String serialData = Serial.readStringUntil('\n');
    tcpClient.println(serialData);
    Serial.println("Transfer to calculator: " + serialData);
  }
}

void loadSettings() {
  EEPROM.get(0, settings);
}

void saveSettings() {
  EEPROM.put(0, settings);
  EEPROM.commit();
}

void resetToFactorySettings() {
  memset(&settings, 0, sizeof(settings));
  EEPROM.put(0, settings);
  EEPROM.commit();
}
