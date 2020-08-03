#include <Arduino.h>
#include <config.h>
#include <secrets.h>

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800    // Modem is SIM800
#define TINY_GSM_RX_BUFFER 1024  // Set RX buffer to 1Kb

#include <TinyGsmClient.h>
#include <Wire.h>

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

TinyGsm modem(SerialAT);

#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00

void updateSerial() {
  while (SerialAT.available()) {
    SerialMon.print("Received: ");
    SerialMon.print(SerialAT.readStringUntil('\n'));
  }
  while (SerialMon.available()) {
    SerialMon.print("Sending : ");
    String msg = SerialMon.readStringUntil('\n');
    SerialMon.print(msg);
    SerialAT.print(msg);
  }
}

void send(const char* msg) {
  updateSerial();
  SerialMon.print("Sending : ");
  SerialMon.println(msg);
  SerialAT.println(msg);
  while (!SerialAT.available()) {
    delay(50);
  }
  updateSerial();
}

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
  SerialMon.println("Initializing modem...");
  modem.factoryDefault();
  modem.restart();
  modem.waitForNetwork();
  modem.getGsmLocation();
  modem.gprsConnect("virgin-internet");

  modem.sendAT(GF("+EMAILCID=1"));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: EMAILCID");
    return;
  }

  modem.sendAT(GF("+EMAILTO=30"));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: EMAILTO");
    return;
  }

  modem.sendAT(GF("+EMAILSSL=1"));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: EMAILSSL");
    return;
  }

  modem.sendAT(GF("+SMTPSRV=\"smtp.wp.pl\""));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPSRV");
    return;
  }

  modem.sendAT(GF("+SMTPAUTH=1,\"" S_SMTP_USER "\",\"" S_SMTP_PASS "\""));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPAUTH");
    return;
  }

  modem.sendAT(GF("+SMTPFROM=\"" S_EMAIL_FROM "\",\"" S_EMAIL_FROM_NAME "\""));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPFROM");
    return;
  }

  modem.sendAT(
      GF("+SMTPRCPT=0,0,\"" S_EMAIL_RCPT "\",\"" S_EMAIL_RCPT_NAME "\""));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPRCPT");
    return;
  }

  modem.sendAT(GF("+SMTPSUB=\"test\""));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPSUB");
    return;
  }

  auto msg = String("teh message loljkkkkk");
  auto len = String(msg.length(), 10);
  modem.sendAT(String("+SMTPBODY=") + len);
  if (modem.waitResponse("DOWNLOAD") != 1) {
    SerialMon.println("Error: SMTPBODY - length");
    return;
  }

  modem.streamWrite(msg, GSM_NL);
  modem.stream.flush();
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPBODY - message");
    return;
  }

  modem.sendAT(GF("+SMTPSEND"));
  if (modem.waitResponse() != 1) {
    SerialMon.println("Error: SMTPSEND");
    return;
  }
}

// unsigned long lastCheck = 0;
void loop() {
  // if (millis() - lastCheck > 10000) {
  //   lastCheck = millis();
  //   auto q = modem.getSignalQuality();
  //   SerialMon.printf("Signal quality: %d\n", q);

  //   auto loc = modem.getGsmLocation();
  //   SerialMon.printf("GSM Location: %s\n", loc.c_str());

  //   auto ip = modem.getLocalIP();
  //   SerialMon.printf("Current IP: %s\n", ip.c_str());

  //   auto oper = modem.getOperator();
  //   SerialMon.printf("Operator: %s\n", oper.c_str());
  // }
  updateSerial();
}
