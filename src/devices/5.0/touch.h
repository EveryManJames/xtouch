#ifndef _XLCD_TOUCH
#define _XLCD_TOUCH

#include <Wire.h>
#include <TAMC_GT911.h>

// GT911 capacitive touch controller (I2C) on the Guition JC8048W550.
// Unlike the 2.8" resistive XPT2046, GT911 reports absolute pixel
// coordinates, so there is NO calibration step and no touch.json.
//
// These match the established vendor-demo config for this board family
// (Sunton 8048S050C / Guition JC8048W550): SDA=19, SCL=20, RST=38, INT
// unconnected. NOTE (building blind, verify on hardware):
//   * If touch is dead, the panel may sit at I2C addr 0x14 instead of 0x5D -
//     change the begin() call in xtouch_touch_setup() to GT911_ADDR2.
//   * INT=-1 is passed to a uint8_t param (becomes 255); the library's one-time
//     reset() will log a harmless "GPIO number error" for it. Set a real INT
//     pin here if your revision wires one.
//   * If X/Y are swapped or mirrored, change ROTATION_NORMAL below.
#define XTOUCH_GT911_SDA 19
#define XTOUCH_GT911_SCL 20
#define XTOUCH_GT911_INT -1
#define XTOUCH_GT911_RST 38
#define XTOUCH_GT911_WIDTH 800
#define XTOUCH_GT911_HEIGHT 480

TAMC_GT911 x_touch_gt911 = TAMC_GT911(
    XTOUCH_GT911_SDA, XTOUCH_GT911_SCL,
    XTOUCH_GT911_INT, XTOUCH_GT911_RST,
    XTOUCH_GT911_WIDTH, XTOUCH_GT911_HEIGHT);

class ScreenPoint
{
public:
    int16_t x;
    int16_t y;

    ScreenPoint() {}
    ScreenPoint(int16_t xIn, int16_t yIn)
    {
        x = xIn;
        y = yIn;
    }
};

// Capacitive coordinates are already in panel space; just clamp to bounds.
ScreenPoint getScreenCoords(int16_t x, int16_t y)
{
    if (x < 0)
        x = 0;
    if (x >= XTOUCH_GT911_WIDTH)
        x = XTOUCH_GT911_WIDTH - 1;
    if (y < 0)
        y = 0;
    if (y >= XTOUCH_GT911_HEIGHT)
        y = XTOUCH_GT911_HEIGHT - 1;
    return ScreenPoint(x, y);
}

// A capacitive panel has nothing to calibrate. Kept for API parity with the
// 2.8" build (referenced by events.h and the flip toggle): drop any stale
// touch.json and reboot.
void xtouch_resetTouchConfig()
{
    ConsoleInfo.println(F("[XTouch][TOUCH] Capacitive panel - no calibration; restarting"));
    xtouch_filesystem_deleteFile(SD, xtouch_paths_touch);
    delay(200);
    ESP.restart();
}

bool hasTouchConfig()
{
    return true;
}

void xtouch_touch_setup()
{
    ConsoleInfo.println(F("[XTouch][TOUCH] GT911 capacitive setup"));
    x_touch_gt911.begin();
    x_touch_gt911.setRotation(ROTATION_NORMAL);
}

#endif
