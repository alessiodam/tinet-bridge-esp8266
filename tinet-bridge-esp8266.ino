#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266httpUpdate.h>

#define WIFI_SSID "ssid-here"
#define WIFI_PASS "password-here"
#define SERIAL_BAUDRATE 115200
#define TINET_HUB_HOST "tinethub.tkbstudios.com"
#define TINET_HUB_PORT 2052

uint8_t YELLOW_LED = D2;
uint8_t BLUE_LED = D5;
uint8_t GREEN_LED = D6;
uint8_t RED_LED = D8;

WiFiClient wifi_client;
int wifi_status = WL_IDLE_STATUS;
WiFiClient tcp_client;

IPAddress cloudflare_dns(1, 1, 1, 1);
IPAddress google_dns(8, 8, 8, 8);

AsyncWebServer server(80);

struct Settings {
  char password[64];
  unsigned long transferred_packets;
  float total_mb;
};

Settings settings;

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  EEPROM.begin(sizeof(Settings));

  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, HIGH);

  delay(1000);

  digitalWrite(BLUE_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  
  loadSettings();

  //WiFi.setDNS(cloudflare_dns, google_dns);
  Serial.println("WIFI_CONNECTING");
  wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (wifi_status != WL_CONNECTED) {
    wifi_status = WiFi.status();
    digitalWrite(GREEN_LED, HIGH);
    delay(100);
    digitalWrite(GREEN_LED, LOW);
    delay(100);
  }
  
  Serial.println("WIFI_CONNECTED");
  digitalWrite(GREEN_LED, LOW);
  flashLED(GREEN_LED, 200);
  digitalWrite(YELLOW_LED, HIGH);
  Serial.println("LOCAL_IP_ADDR:" + WiFi.localIP().toString());
  
  setupAsyncServer();
}

void loop() {
  if (tcp_client.connected()) {
    Serial.println("TCP_CONNECTED");
  } else if (Serial.available() && !tcp_client.connected()) {
    Serial.println("wait for calc");
    handleSerialToTCP();
  }
}

void setupAsyncServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    flashLED(GREEN_LED, 10);
    if (strlen(settings.password) == 0) {
      request->send(200, "text/html", "Please set a password <a href='/setpassword'>here</a>.");
    } else {
      String html = "<html><body>";
      html += "<h2>Management Page</h2>";
      html += "<p>Transferred Packets: " + String(settings.transferred_packets) + "</p>";
      html += "<p>Total Megabytes: " + String(settings.total_mb) + "</p>";
      html += "<form action='/setpassword' method='post'>";
      html += "<label>Password (please do this on your local network for better security! Max 64 chars.): </label>";
      html += "<input type='password' name='password'/>";
      html += "<input type='submit' value='Set'/></form>";
      html += "<form action='/reset' method='post'>";
      html += "<input type='submit' value='Reset to Factory Settings' onclick='return confirm(\"Are you sure?\");'/></form>";
      html += "</body></html>";
      request->send(200, "text/html", html);
    }
  });

  server.on("/setpassword", HTTP_GET, [](AsyncWebServerRequest *request){
    flashLED(GREEN_LED, 10);
    request->send(200, "text/html", "<html><body><form action='/savepassword' method='post'><label>Password: </label><input type='password' name='password'/><input type='submit' value='Set'/></form></body></html>");
  });

  server.on("/savepassword", HTTP_POST, [](AsyncWebServerRequest *request){
    flashLED(GREEN_LED, 10);
    if (request->hasParam("password", true)) {
      String newPassword = request->getParam("password", true)->value();
      newPassword.toCharArray(settings.password, sizeof(settings.password));
      saveSettings();
    }
    request->send(200, "text/html", "Password set successfully. <a href='/'>Go to Management Page</a>");
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    flashLED(GREEN_LED, 10);
    digitalWrite(RED_LED, HIGH);
    resetToFactorySettings();
    delay(1000);
    digitalWrite(RED_LED, LOW);
    request->send(200, "text/html", "Reset to factory settings successful. <a href='/'>Go to Management Page</a>");
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    // IN DEVELOPMENT
    digitalWrite(BLUE_LED, HIGH);
    delay(250);
    digitalWrite(GREEN_LED, HIGH);
    delay(250);
    digitalWrite(YELLOW_LED, HIGH);
    delay(250);
    digitalWrite(RED_LED, HIGH);
    delay(250);

    Serial.println("BRIDGE_UPDATING");

    t_httpUpdate_return update_ret = ESPhttpUpdate.update(wifi_client, "github.com", 443, "/tkbstudios/tinet-bridge-esp8266/releases/download/latest/tinet-bridge-esp8266.bin");
    switch (update_ret) {
      case HTTP_UPDATE_FAILED:
        Serial.println("BRIDGE_UPDATE_FAILED");
        request->send(200, "text/html", "Update failed.");
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("BRIDGE_NO_UPDATES_AVAILABLE");
        request->send(200, "text/html", "No updates available!");
        break;
      case HTTP_UPDATE_OK:
        // might not get called because the update function reboots the ESP..
        Serial.println("BRIDGE_UPDATE_SUCCESS");
        request->send(200, "text/html", "Update success!");
        break;
      default:
        Serial.println("BRIDGE_UPDATE_FAILED");
        request->send(200, "text/html", "Update failed.");
        break;
    }
  });

  server.begin();
}

void handleTCPToSerial() {
  if (wifi_client.available() && Serial.available()) {
    String serial_data = Serial.readStringUntil('\n');
    int bytes_read = 0;
    settings.transferred_packets++;
    settings.total_mb += bytes_read / 1024.0 / 1024.0;    
    // show the user that data is being transferred
    flashLED(BLUE_LED, 10);
  }
}

void handleSerialToTCP() {
  if (Serial.available()) {
    String serial_data = Serial.readStringUntil('\n');
    // transfer data over
    // show the user that data is being transferred
    flashLED(BLUE_LED, 10);
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

void flashLED(uint8_t led_to_flash, int delay_to_flash) {
  digitalWrite(led_to_flash, HIGH);
  delay(delay_to_flash);
  digitalWrite(led_to_flash, LOW);
}
