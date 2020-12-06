// Config must be included first
#include "config.h"
// Config must be included first

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
// #define TINY_GSM_DEBUG SerialMon

void sendHeartBeatIfNecessary();
#define TINY_GSM_YIELD()        \
  {                             \
    sendHeartBeatIfNecessary(); \
    delay(1);                   \
  }

#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <secrets.h>
#include <sendmail.h>

// #define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37);  // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35);  // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}

const String doGet(HttpClient* http, const String& path) {
  SerialMon.print("GET ");
  SerialMon.print(path);
  SerialMon.print(" [");
  SerialMon.print(http->get(path));
  SerialMon.print(" ");
  SerialMon.print(http->responseStatusCode());
  SerialMon.println("]");
  return http->responseBody();
}

WiFiClient hbClient;
unsigned long lastHB = 0;
void sendHeartBeatIfNecessary() {
  if (!hbClient.connected()) {
    SerialMon.print("Setting up cam's HeartBeat connection");
    hbClient.connect(S_CAM_IP, 3333);
    while (!hbClient.connected()) {
      SerialMon.print('.');
      delay(500);
    }
    SerialMon.println("\nCam's HeartBeat connection established");
  }

  if (millis() - lastHB > 4000) {
    lastHB = millis();
    hbClient.write("02:001:0");
    hbClient.flush();
  }
}

void skipAtStreamUntil(char c) {
  while (1) {
    if (SerialAT.read() == c) {
      return;
    }
    delay(10);
    sendHeartBeatIfNecessary();
  }
}

const String getMailBody() {
  modem.sendAT(GF("+CSCS=\"GSM\""));
  if (modem.waitResponse() != 1) {
    skipAtStreamUntil('\n');
    return "";
  }
  modem.sendAT(GF("+CUSD=1,\"*101#\",15"));
  if (modem.waitResponse(30000, "+CUSD: 0, \"") != 1) {
    skipAtStreamUntil('\n');
    return "";
  }

  auto body = modem.stream.readStringUntil('"');
  SerialMon.println(body);
  skipAtStreamUntil('\n');
  return body;
}

void setup() {
  SerialMon.begin(115200);

  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  setPowerBoostKeepOn(1);

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(6000);

  if (!WiFi.isConnected()) {
    SerialMon.print("Setting up WiFi connection");
    WiFi.begin(S_CAM_SSID, S_CAM_PASS);
    while (!WiFi.isConnected()) {
      SerialMon.print('.');
      delay(500);
    }
    SerialMon.println("\nWiFi connection established");
  }

  sendHeartBeatIfNecessary();

  WiFiClient camClient;
  if (!camClient.connected()) {
    SerialMon.print("Setting up cam's API connection");
    camClient.connect(S_CAM_IP, 80);
    while (!camClient.connected()) {
      SerialMon.print('.');
      delay(500);
    }
    SerialMon.println("\nCam's API connection established");
  }

  HttpClient camHttp(camClient, S_CAM_IP);
  camHttp.connectionKeepAlive();

  doGet(&camHttp, "/?custom=1&" C_YI_CONNECT);
  sendHeartBeatIfNecessary();

  doGet(&camHttp, "/?custom=1&" C_YI_VIDEO_RECORD_OFF);
  sendHeartBeatIfNecessary();

  doGet(&camHttp, "/?custom=1&" C_YI_MODE_VIDEO);
  sendHeartBeatIfNecessary();

  doGet(&camHttp, "/?custom=1&" C_YI_VIDEO_RECORD_OFF);
  sendHeartBeatIfNecessary();

  doGet(&camHttp, "/?custom=1&" C_YI_VIDEO_AUTO_START_OFF);
  sendHeartBeatIfNecessary();

  doGet(&camHttp, "/?custom=1&" C_YI_PHOTO_RESOLUTION1280);

  do {
    sendHeartBeatIfNecessary();
    SerialMon.println("Restarting modem... ");
    modem.restart();
    sendHeartBeatIfNecessary();
    modem.waitForNetwork();
    sendHeartBeatIfNecessary();

    SerialMon.println("Initializing GPRS... ");
  } while (!modem.gprsConnect("virgin-internet"));
  sendHeartBeatIfNecessary();

  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  if (modem.getNetworkTime(&year, &month, &day, &hour, &minute, &second,
                           nullptr)) {
    sendHeartBeatIfNecessary();
    char timeBuf[30];
    //                     yyyy- mm - dd _ HH : mm :  ss
    sprintf(timeBuf, "&str=%04d-%02d-%02d_%02d:%02d:%02d", year, month, day,
            hour, minute, second);
    String timeCmd = "/?custom=1&" C_YI_CLOCK;
    doGet(&camHttp, timeCmd + timeBuf);
    sendHeartBeatIfNecessary();
  }

  doGet(&camHttp, "/?custom=1&" C_YI_MODE_PHOTO);
  sendHeartBeatIfNecessary();

  auto res = doGet(&camHttp, "/?custom=1&" C_YI_TAKE_PHOTO);
  sendHeartBeatIfNecessary();
  auto start = res.indexOf("<FPATH>A:") + 9;
  auto end = res.indexOf("</FPATH", start);
  if (start == -1 || end == -1) {
    SerialMon.println("Couldn't find path in response:");
    SerialMon.println(res);
    return;
  }

  auto path = res.substring(start, end);
  path.replace('\\', '/');

  doGet(&camHttp, "/?custom=1&" C_YI_MODE_FILE);
  sendHeartBeatIfNecessary();

  auto mailBody = getMailBody();
  sendHeartBeatIfNecessary();
  sendMail(modem, mailBody, &camHttp, path);

  camHttp.stop();
}

void loop() {
  delay(100);
}