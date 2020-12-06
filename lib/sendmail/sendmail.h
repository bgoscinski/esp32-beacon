#pragma once

// Config must be included first
#include "config.h"
// Config must be included first

#include <TinyGsmClient.h>
#include <b64.h>
#include <secrets.h>

#define EXPECT(x, ret) \
  if (!(x))            \
    return (ret);

inline int16_t streamGetIntBefore(TinyGsm& modem, char lastChar) {
  char buf[7];
  size_t bytesRead =
      modem.stream.readBytesUntil(lastChar, buf, static_cast<size_t>(7));
  // if we read 7 or more bytes, it's an overflow
  if (bytesRead && bytesRead < 7) {
    buf[bytesRead] = '\0';
    int16_t res = atoi(buf);
    return res;
  }

  return -9999;
}

#define ENC_SIZE(size) (((size) + 2) / 3) << 2
#define DEC_SIZE(size) (((size) >> 2) * 3)
#define BUF_SIZE (size_t)3 * 512
#define ENC_BUF_SIZE ENC_SIZE(BUF_SIZE)
unsigned char buf[BUF_SIZE];
unsigned char encBuf[ENC_BUF_SIZE];

void printProgress(int len, int pos) {
  String pct = len <= 0 ? "?" : String((100 * pos) / len);
  SerialMon.print("Transferring photo... ");
  SerialMon.print(pct);
  SerialMon.print("% (");
  SerialMon.print(pos);
  SerialMon.print(" / ");
  SerialMon.print(len);
  SerialMon.println(" B)");
}

bool seekAtt(HttpClient* att, const String& path, int seekBytes, int* len) {
  att->get(path);
  att->responseStatusCode();

  // don't use att->contentLength() because it's case sensitive
  while (!att->endOfHeadersReached() && att->headerAvailable()) {
    auto name = att->readHeaderName();
    if (name.equalsIgnoreCase("content-length")) {
      *len = att->readHeaderValue().toInt();
      att->skipResponseHeaders();
    }
  }

  auto lastRead = millis();
  seekBytes = min(seekBytes, *len);

  SerialMon.print("Attempting to seek through ");
  SerialMon.print(seekBytes);
  SerialMon.print(" of ");
  SerialMon.print(*len);
  SerialMon.println(" bytes");

  int c;
  for (int i = 0; i < seekBytes; i++) {
    sendHeartBeatIfNecessary();
    while ((c = att->read()) == -1) {
      delay(100);
      if ((millis() - lastRead) > 3000) {
        return false;
      }
    }
    if (i < 100 || (seekBytes - i) < 100) {
      SerialMon.printf("%02x ", c);
    } else if (i == 100 && (seekBytes - i) > 100) {
      SerialMon.print(".. ");
    }
    lastRead = millis();
  }
  SerialMon.println();

  return true;
}

/**
 * value guaranteed to be 3*n
 */
size_t getToRead(TinyGsm& modem) {
  EXPECT(modem.waitResponse(3 * 60 * 1000, "+SMTPFT: 1,") == 1, false)
  size_t maxSize = streamGetIntBefore(modem, '\n');
  size_t maxEncSize = maxSize - (maxSize % 4);
  return min(BUF_SIZE, DEC_SIZE(maxEncSize));
}

void awaitCamData(HttpClient* att, const String& path, int pos, int* len) {
  auto waitForCamDataStart = millis();
  do {
    delay(50);
    sendHeartBeatIfNecessary();
    if ((millis() - waitForCamDataStart) > 1000) {
      SerialMon.println("Lost connection. Resurrecting it...");
      seekAtt(att, path, pos, len);
      waitForCamDataStart = millis();
    }
  } while (pos < *len && !att->available());
}

bool sendMail(TinyGsm& modem,
              const String& message,
              HttpClient* att,
              const String& path) {
  modem.sendAT(GF("+EMAILCID=1"));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+EMAILTO=30"));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+EMAILSSL=1"));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+SMTPSRV=\"smtp.wp.pl\""));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+SMTPAUTH=1,\"" S_SMTP_USER "\",\"" S_SMTP_PASS "\""));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+SMTPFROM=\"" S_EMAIL_FROM "\",\"" S_EMAIL_FROM_NAME "\""));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(
      GF("+SMTPRCPT=0,0,\"" S_EMAIL_RCPT "\",\"" S_EMAIL_RCPT_NAME "\""));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+SMTPSUB=\"Fotka\""));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+SMTPBODY="), String(message.length(), 10));
  EXPECT(modem.waitResponse("DOWNLOAD") == 1, false)

  modem.streamWrite(message, GSM_NL);
  modem.stream.flush();
  EXPECT(modem.waitResponse() == 1, false)

  if (att) {
    String name = path.substring(path.lastIndexOf('/') + 1);
    modem.sendAT(GF("+SMTPFILE=2,\""), name, GF("\",1"));
    EXPECT(modem.waitResponse() == 1, false)
  }

  modem.sendAT(GF("+SMTPSEND"));
  EXPECT(modem.waitResponse() == 1, false)

  if (att) {
    int pos = 0;
    int len = -1;

    // get len, start the connection
    seekAtt(att, path, 0, &len);

    while (1) {
      if (pos == len) {
        SerialMon.println("Transfer complete");
        break;
      }

      printProgress(len, pos);
      awaitCamData(att, path, pos, &len);

      auto toRead = getToRead(modem);
      auto readBytes = att->readBytes(buf, toRead);
      pos += readBytes;

      while (readBytes % 3 && readBytes < toRead && pos < len) {
        SerialMon.print("Aligning data to encode to be a multiply of 3... ");
        awaitCamData(att, path, pos, &len);
        if (pos < len) {
          SerialMon.println("+1 B");
          buf[readBytes++] = att->read();
          pos++;
        } else {
          SerialMon.println("Reached EOF");
          break;
        }
      }

      auto encSize = b64_encode(buf, readBytes, encBuf, ENC_BUF_SIZE);
      modem.sendAT(GF("+SMTPFT="), String(encSize));
      EXPECT(modem.waitResponse("+SMTPFT: 2,") == 1, false)
      EXPECT(modem.waitResponse(String(encSize).c_str()) == 1, false)

      modem.stream.write(encBuf, encSize);
      modem.stream.flush();

      EXPECT(modem.waitResponse(3 * 60 * 1000) == 1, false)
    }

    EXPECT(modem.waitResponse("+SMTPFT: 1,") == 1, false)
    streamGetIntBefore(modem, '\n');
    modem.sendAT(GF("+SMTPFT=0"));
    EXPECT(modem.waitResponse() == 1, false)
  }

  SerialMon.println("Mail sent");
  return true;
}
