#pragma once

// Config must be included first
#include "config.h"
// Config must be included first

#include <TinyGsmClient.h>
#include <secrets.h>

#define EXPECT(x, ret) \
  if (!(x))            \
    return (ret);

bool sendTextMail(TinyGsm& modem, String& message) {
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

  modem.sendAT(GF("+SMTPSUB=\"test\""));
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT("+SMTPBODY=", String(message.length(), 10));
  EXPECT(modem.waitResponse("DOWNLOAD") == 1, false)

  modem.streamWrite(message, GSM_NL);
  modem.stream.flush();
  EXPECT(modem.waitResponse() == 1, false)

  modem.sendAT(GF("+SMTPSEND"));
  EXPECT(modem.waitResponse() == 1, false)

  return true;
}