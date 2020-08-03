// Config must be included first
#include "config.h"
// Config must be included first

#include <Arduino.h>
#include <HTTPClient.h>
#include <TinyGsmClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <secrets.h>
#include <sendmail.h>

HTTPClient http;
WiFiClient hbClient;

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

TinyGsm modem(SerialAT);

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

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);

  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  // SerialMon.println("Initializing modem...");
  // modem.factoryDefault();
  // modem.restart();
  // modem.waitForNetwork();
  // modem.getGsmLocation();
  // modem.gprsConnect("virgin-internet");

  // auto msg = String("this is a message\nsecond line");
  // sendTextMail(modem, msg);

  SerialMon.print("Setting up WiFi connection");
  WiFi.begin(S_CAM_SSID, S_CAM_PASS);
  while (!WiFi.isConnected()) {
    SerialMon.print('.');
    delay(500);
  }
  SerialMon.println("\nWiFi connection established");

  SerialMon.print("Setting up H connection");
  hbClient.connect(S_CAM_IP, 3333);
}

unsigned long lastHB = 0;
void loop() {
  if (millis() - lastHB > 5000) {
  }
  while (SerialAT.available()) {
    SerialMon.write(SerialAT.read());
  }
  while (SerialMon.available()) {
    SerialAT.write(SerialMon.read());
  }
}
