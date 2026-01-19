#pragma once

#include <switch.h>

extern PadState g_pad;

#define HidNpadButton_Any 0x0000000FFFFFFFFF

typedef struct {
    s32 px;
    s32 py;
    s32 dx;
    s32 dy;
    s32 angle;
} touchPosition;

#define KEY_LSTICK_LEFT  BITL(16)
#define KEY_LSTICK_UP    BITL(17)
#define KEY_LSTICK_RIGHT BITL(18)
#define KEY_LSTICK_DOWN  BITL(19)
#define KEY_RSTICK_LEFT  BITL(20)
#define KEY_RSTICK_UP    BITL(21)
#define KEY_RSTICK_RIGHT BITL(22)
#define KEY_RSTICK_DOWN  BITL(23)

#define HidNpadButton_AnyLeft  (HidNpadButton_Left | HidNpadButton_StickLLeft | HidNpadButton_StickRLeft)
#define HidNpadButton_AnyUp    (HidNpadButton_Up | HidNpadButton_StickLUp | HidNpadButton_StickRUp)
#define HidNpadButton_AnyRight (HidNpadButton_Right | HidNpadButton_StickLRight | HidNpadButton_StickRRight)
#define HidNpadButton_AnyDown  (HidNpadButton_Down | HidNpadButton_StickLDown | HidNpadButton_StickRDown)

/* || EDIZON COMPILE TIME SETTINGS || */

/* The EdiZon folder base path on the SD card */
#define EDIZON_DIR "/switch/EdiZon"

/* Whether the splash screen should be displayed on launch */
#define SPLASH_ENABLED 1

/* Current web API version */
#define API_VERSION "v3"

/* Web API URL */
#define EDIZON_URL "http://api.edizon.werwolv.net/" API_VERSION 
// #define AMS_URL     "https://api.github.com/repos/tomvita/EdiZon-SE/releases"
// #define EDIZON_URL  "https://api.github.com/repos/tomvita/EdiZon-SE/releases/latest"
// #define APP_URL     "https://github.com/tomvita/EdiZon-SE/releases/latest/download/EdiZon.nro"