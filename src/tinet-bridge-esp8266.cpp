#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <CertStoreBearSSL.h>

#include "htmls.h"

#define SERIAL_BAUDRATE 115200
#define TINET_HUB_HOST "tinethub.tkbstudios.com"
#define TINET_HUB_PORT 2052
#define UPDATE_URL "https://github.com/tkbstudios/tinet-bridge-esp8266/releases/latest/download/firmware.bin"

#define NO_OTA_NETWORK

const uint8_t YELLOW_LED = 5; // D1 on NodeMCU
const uint8_t BLUE_LED = 4;   // D2 on NodeMCU
const uint8_t GREEN_LED = 14; // D5 on NodeMCU
const uint8_t RED_LED = 12;   // D6 on NodeMCU

WiFiClient wifi_client;
int wifi_status = WL_IDLE_STATUS;
WiFiClient tcp_client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BearSSL::WiFiClientSecure wifi_update_client;
BearSSL::CertStore certStore;

IPAddress cloudflare_dns(1, 1, 1, 1);
IPAddress google_dns(8, 8, 8, 8);

ESP8266WebServer server(80);

const byte DNS_PORT = 53;
DNSServer dnsServer;

IPAddress apIP(192, 168, 1, 1);
IPAddress netMsk(255, 255, 255, 0);

struct Settings
{
  char wifi_ssid[32];
  char wifi_pass[64];
  char password[64];
  long utc_offset_seconds;
  bool boot_setup_mode;
};

Settings settings;

void handleSetupRoot();
void handleSetupSaveConfig();
void handleReset();
void handleRoot();
void handleSetPasswordPage();
void handleSavePassword();
void handleUpdate();
bool connectToTCPServer();
void handleTCPToSerial();
void handleSerialToTCP();
void loadSettings();
void saveSettings();
void resetToFactorySettings();
void flashLED(uint8_t led_to_flash, int delay_to_flash);

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  Serial.setTimeout(1000);
  EEPROM.begin(sizeof(Settings));
  wifi_update_client.setInsecure();

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

  float eepromdataaddresszero;
  EEPROM.get(0, eepromdataaddresszero);

  if (strlen(settings.wifi_ssid) == 0 || strlen(settings.wifi_pass) == 0 || isnan(eepromdataaddresszero) || settings.boot_setup_mode == true)
  {
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("BRIDGE_SET_UP_WIFI");

    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP("TINETbridge", "12345678");

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

    server.on("/", HTTP_GET, handleSetupRoot);
    server.on("/saveconfig", HTTP_POST, handleSetupSaveConfig);
    server.on("/reset", HTTP_POST, handleReset);
    server.on("/generate_204", handleSetupRoot); // Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", handleSetupRoot);       // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.onNotFound(handleSetupRoot);          // idk testing things out :p

    if (!MDNS.begin("tinetbridge"))
    {
      flashLED(RED_LED, 500);
    }
    else
    {
      MDNS.addService("http", "tcp", 80);
    }

    server.begin();

    while (strlen(settings.wifi_ssid) == 0 || strlen(settings.wifi_pass) == 0 || isnan(eepromdataaddresszero) || settings.boot_setup_mode == true)
    {
      dnsServer.processNextRequest();
      server.handleClient();
    }
  }

  Serial.println("WIFI_CONNECTING");
  unsigned long wifi_connect_start_time = millis();
  unsigned long wifi_connect_current_time = millis();
  unsigned long wifi_connect_elapsed_time = 0;
  wifi_status = WiFi.begin(settings.wifi_ssid, settings.wifi_pass);

  while (wifi_status != WL_CONNECTED)
  {
    wifi_status = WiFi.status();
    digitalWrite(GREEN_LED, HIGH);
    delay(100);
    digitalWrite(GREEN_LED, LOW);
    delay(100);
    wifi_connect_current_time = millis();
    wifi_connect_elapsed_time = wifi_connect_current_time - wifi_connect_start_time;
    if (wifi_connect_elapsed_time >= 10000)
    {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Serial.println("BRIDGE_REBOOT_TO_SETUP");
      settings.boot_setup_mode = true;
      saveSettings();

      delay(2000);
      ESP.restart();
    }
  }

  Serial.println("WIFI_CONNECTED");
  digitalWrite(GREEN_LED, LOW);
  flashLED(GREEN_LED, 200);
  digitalWrite(YELLOW_LED, HIGH);
  Serial.println("LOCAL_IP_ADDR:" + WiFi.localIP().toString());

  timeClient.begin();
  timeClient.update();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/setpassword", HTTP_GET, handleSetPasswordPage);
  server.on("/savepassword", HTTP_POST, handleSavePassword);
  server.on("/reset", HTTP_POST, handleReset);
  server.on("/update", HTTP_POST, handleUpdate);
  server.begin();

  connectToTCPServer();
}

void loop()
{
  server.handleClient();
  if (tcp_client.connected())
  {
    handleSerialToTCP();
    handleTCPToSerial();
  }
  else if (!tcp_client.connected())
  {
    handleSerialToTCP();
  }
}

void handleSetupRoot()
{
  flashLED(GREEN_LED, 10);

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  server.send(200, "text/html", SETUP_ROOT_PAGE_HTML);
}

void handleSetupSaveConfig()
{
  flashLED(GREEN_LED, 10);

  String newSSID = server.arg("ssid").c_str();
  String newPassword = server.arg("password").c_str();
  ;
  newSSID.toCharArray(settings.wifi_ssid, sizeof(settings.wifi_ssid));
  newPassword.toCharArray(settings.wifi_pass, sizeof(settings.wifi_pass));
  settings.boot_setup_mode = false;

  saveSettings();

  server.send(200, "text/html", SETUP_SAVE_CONFIG_HTML);
  delay(200);
  ESP.restart();
}

void handleReset()
{
  flashLED(GREEN_LED, 10);
  digitalWrite(RED_LED, HIGH);
  resetToFactorySettings();
  delay(1000);
  digitalWrite(RED_LED, LOW);
  server.send(200, "text/html", RESET_HTML);
  delay(200);
  ESP.restart();
}

void handleRoot()
{
  flashLED(GREEN_LED, 10);
  if (strlen(settings.password) == 0)
  {
    server.send(200, "text/html", ROOT_NO_PASSWORD_HTML);
  }
  else
  {
    server.send(200, "text/html", ROOT_HTML);
  }
}

void handleSetPasswordPage()
{
  flashLED(GREEN_LED, 10);
  server.send(200, "text/html", SET_PASSWORD_HTML);
}

void handleSavePassword()
{
  flashLED(GREEN_LED, 10);
  String newPassword = server.arg("password").c_str();
  newPassword.toCharArray(settings.password, sizeof(settings.password));
  saveSettings();
  server.send(200, "text/html", SET_PASSWORD_SUCCESS_HTML);
}

void handleUpdate()
{
  // IN DEVELOPMENT
  Serial.println("BRIDGE_UPDATING");

  digitalWrite(BLUE_LED, HIGH);
  delay(250);
  digitalWrite(GREEN_LED, HIGH);
  delay(250);
  digitalWrite(YELLOW_LED, HIGH);
  delay(250);
  digitalWrite(RED_LED, HIGH);
  delay(250);

  ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  ESPhttpUpdate.setLedPin(BLUE_LED, true);
  ESPhttpUpdate.rebootOnUpdate(false);

  t_httpUpdate_return update_ret = ESPhttpUpdate.update(wifi_update_client, UPDATE_URL);

  switch (update_ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.println("BRIDGE_UPDATE_FAILED");
    Serial.println(ESPhttpUpdate.getLastErrorString().c_str());
    server.send(200, "text/html", UPDATE_FAILED_HTML);
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("BRIDGE_NO_UPDATES_AVAILABLE");
    server.send(200, "text/html", NO_UPDATES_AVAILABLE_HTML);
    break;
  case HTTP_UPDATE_OK:
    Serial.println("BRIDGE_UPDATE_SUCCESS");
    server.send(200, "text/html", UPDATE_SUCCESS_HTML);
    delay(200);
    ESP.restart();
    break;
  }
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

bool connectToTCPServer()
{
  unsigned long tcp_connect_start_time = millis();
  unsigned long tcp_connect_current_time = millis();
  unsigned long tcp_connect_elapsed_time = 0;
  tcp_client.connect(TINET_HUB_HOST, TINET_HUB_PORT);
  while (!tcp_client.connected())
  {
    tcp_connect_elapsed_time = tcp_connect_current_time - tcp_connect_start_time;
    if (tcp_connect_elapsed_time > 5000)
    {
      // timed out
      return false;
    }
  }
  return true;
}

void handleTCPToSerial()
{
  if (tcp_client.connected())
  {
    int available_in_tcp = tcp_client.available();

    if (available_in_tcp > 0)
    {
      flashLED(BLUE_LED, 10);

      while (tcp_client.available())
      {
        char c = tcp_client.read();
        Serial.write(c);
      }
    }
  }
}

void handleSerialToTCP()
{
  if (Serial.available() != 0)
  {
    flashLED(BLUE_LED, 10);
    String serial_data = Serial.readStringUntil('\n');

    if (!tcp_client.connected() && serial_data == "CONNECT_TCP")
    {
      if (connectToTCPServer())
      {
        Serial.println("TCP_CONNECTED");
      }
      else
      {
        Serial.println("TCP_CONNECT_TIMED_OUT");
      }
    }
    else if (serial_data == "GET_TIME")
    {
      Serial.print("CURRENT_TIME:");
      Serial.println(timeClient.getFormattedTime());
    }
    else if (serial_data == "GET_LOCAL_IP_ADDRESS")
    {
      Serial.print("LOCAL_IP_ADDR:");
      Serial.println(WiFi.localIP().toString());
    }
    else if (tcp_client.connected())
    {
      tcp_client.println(serial_data);
    }
    else if (tcp_client.connected() && serial_data == "CONNECT_TCP")
    {
      Serial.println("TCP_ALREADY_CONNECTED");
    }

    serial_data = "";
  }
}

void loadSettings()
{
  EEPROM.get(0, settings);
}

void saveSettings()
{
  EEPROM.put(0, settings);
  EEPROM.commit();
}

void resetToFactorySettings()
{
  memset(&settings, 0, sizeof(settings));
  EEPROM.put(0, settings);
  EEPROM.commit();
}

void flashLED(uint8_t led_to_flash, int delay_to_flash)
{
  digitalWrite(led_to_flash, HIGH);
  delay(delay_to_flash);
  digitalWrite(led_to_flash, LOW);
}
