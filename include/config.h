#pragma once

// TTGO T-Call pins
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22
#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800    // Modem is SIM800
#define TINY_GSM_RX_BUFFER 1024  // Set RX buffer to 1Kb

#define C_YI_CARD_INFO "cmd=3039"
#define C_YI_CLOCK "cmd=3034"
#define C_YI_CONFIG "cmd=3014"
#define C_YI_CONNECT "cmd=8001"
#define C_YI_DISCONNECT "cmd=8002"
#define C_YI_FILE_DELETE "cmd=4003"
#define C_YI_FILE_FORCE_DELETE "cmd=4009"
#define C_YI_FILE_LIST "cmd=3015"
#define C_YI_FILE_THUMBNAIL "cmd=4001"
#define C_YI_MODE_PHOTO "cmd=3001&par=0"
#define C_YI_MODE_VIDEO "cmd=3001&par=1"
#define C_YI_MODE_FILE "cmd=3001&par=2"
#define C_YI_TAKE_PHOTO "cmd=1001"
#define C_YI_VIDEO_EMERGENCY "cmd=2019"
#define C_YI_VIDEO_PHOTO "cmd=2017"
#define C_YI_VIDEO_RECORD_OFF "cmd=2001&par=0"
#define C_YI_VIDEO_RECORD_ON "cmd=2001&par=1"
#define C_YI_VIDEO_SECONDS_LEFT "cmd=2009"
#define C_YI_VIDEO_STATE "cmd=2016"
#define C_YI_VIDEO_STREAM_OFF "cmd=2015&par=0"
#define C_YI_VIDEO_STREAM_ON "cmd=2015&par=1"
#define C_YI_ADAS "cmd=2031"
#define C_YI_AUDIO "cmd=2007"
#define C_YI_BUTTON_SOUND "cmd=3041"
#define C_YI_DRIVING_REPORT "cmd=3044"
#define C_YI_EXPOSURE "cmd=2005"
#define C_YI_FIRMWARE_VERSION "cmd=3012"
#define C_YI_GSENSOR "cmd=2011"
#define C_YI_LANGUAGE "cmd=3008"
#define C_YI_MODEL "cmd=3035"
#define C_YI_PHOTO_RESOLUTION640 "cmd=1002&par=5"
#define C_YI_PHOTO_RESOLUTION1280 "cmd=1002&par=6"
#define C_YI_PHOTO_RESOLUTION1920 "cmd=1002&par=7"
#define C_YI_PHOTO_RESOLUTION2048 "cmd=1002&par=4"
#define C_YI_PHOTO_RESOLUTION2592 "cmd=1002&par=3"
#define C_YI_PHOTO_RESOLUTION3264 "cmd=1002&par=2"
#define C_YI_PHOTO_RESOLUTION3648 "cmd=1002&par=1"
#define C_YI_PHOTO_RESOLUTION4032 "cmd=1002&par=0"
#define C_YI_POWER_ON_OFF_SOUND "cmd=2051"
#define C_YI_SERIAL_NUMBER "cmd=3037"
#define C_YI_STANDBY_CLOCK "cmd=2050"
#define C_YI_STANDBY_TIMEOUT "cmd=3033"
#define C_YI_VIDEO_AUTO_START_OFF "cmd=2012&par=0"
#define C_YI_VIDEO_AUTO_START_ON "cmd=2012&par=1"
#define C_YI_VIDEO_LENGTH "cmd=2003"
#define C_YI_VIDEO_LOGO "cmd=2040"
#define C_YI_VIDEO_RESOLUTION "cmd=2002"
#define C_YI_VIDEO_TIMESTAMP "cmd=2008"