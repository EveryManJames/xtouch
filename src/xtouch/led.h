#ifndef _XLCD_LED
#define _XLCD_LED

// Optional user-attached addressable RGB strip (WS2812 / NeoPixel), 5" build.
//
// The JC8048W550 breaks out GPIO 17 and GPIO 18 (plus GND and 3V3) on its
// "Extended IO" JST connectors, and neither is used by the RGB panel, GT911
// touch, or the SD card - so a strip's data line goes to one of those (default
// GPIO 18). Power a strip of more than a few LEDs from an external 5V supply
// with a common ground; the 3V3 rail on the connector is only good for a
// handful of pixels, and a 3.3V->5V data level shifter is recommended for long
// runs.
//
// Config lives in xTouchConfig (persisted in settings.json). The Accessories
// screen mutates it and sends XTOUCH_SETTINGS_LED_APPLY; this module reapplies.

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel xtouch_led_strip;

static bool xtouch_led_ready = false;
static uint8_t xtouch_led_activeGpio = 0;
static uint16_t xtouch_led_activeCount = 0;
static unsigned long xtouch_led_lastTick = 0;
static uint8_t xtouch_led_phase = 0; // breathe / pulse animation phase

static uint32_t xtouch_led_scale(uint32_t rgb, uint8_t s)
{
    uint8_t r = ((rgb >> 16) & 0xFF) * s / 255;
    uint8_t g = ((rgb >> 8) & 0xFF) * s / 255;
    uint8_t b = (rgb & 0xFF) * s / 255;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Color for the status-sync mode based on the current print state.
static uint32_t xtouch_led_statusColor()
{
    switch (bambuStatus.print_status)
    {
    case XTOUCH_PRINT_STATUS_RUNNING:
    case XTOUCH_PRINT_STATUS_PREPARE:
        return 0x46D19E; // running (green)
    case XTOUCH_PRINT_STATUS_PAUSED:
        return 0xFFC24B; // paused (amber)
    case XTOUCH_PRINT_STATUS_FINISHED:
        return 0x46D19E; // finished (green)
    case XTOUCH_PRINT_STATUS_FAILED:
        return 0xFF5D5D; // error (red)
    default:
        return xTouchConfig.xTouchLedColor; // idle -> user color
    }
}

static void xtouch_led_fill(uint32_t rgb)
{
    uint8_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
    xtouch_led_strip.fill(xtouch_led_strip.Color(r, g, b), 0, xtouch_led_strip.numPixels());
}

// (Re)configure the strip from xTouchConfig. Safe to call repeatedly.
void xtouch_led_apply()
{
    if (!xTouchConfig.xTouchLedEnabled || xTouchConfig.xTouchLedCount == 0)
    {
        if (xtouch_led_ready)
        {
            xtouch_led_strip.clear();
            xtouch_led_strip.show();
        }
        return;
    }

    // (Re)init if pin or length changed.
    if (!xtouch_led_ready ||
        xtouch_led_activeGpio != xTouchConfig.xTouchLedGpio ||
        xtouch_led_activeCount != xTouchConfig.xTouchLedCount)
    {
        xtouch_led_strip.updateType(NEO_GRB + NEO_KHZ800);
        xtouch_led_strip.updateLength(xTouchConfig.xTouchLedCount);
        xtouch_led_strip.setPin(xTouchConfig.xTouchLedGpio);
        xtouch_led_strip.begin();
        xtouch_led_activeGpio = xTouchConfig.xTouchLedGpio;
        xtouch_led_activeCount = xTouchConfig.xTouchLedCount;
        xtouch_led_ready = true;
    }

    xtouch_led_strip.setBrightness(xTouchConfig.xTouchLedBrightness);

    switch (xTouchConfig.xTouchLedMode)
    {
    case 0: // off
        xtouch_led_strip.clear();
        break;
    case 1: // solid
    case 2: // breathe (animated in loop; seed solid here)
        xtouch_led_fill(xTouchConfig.xTouchLedColor);
        break;
    case 3: // progress (drawn in loop)
    case 4: // status-sync (drawn in loop)
        xtouch_led_fill(xtouch_led_statusColor());
        break;
    }
    xtouch_led_strip.show();
}

static void xtouch_led_onApply(lv_msg_t *m)
{
    LV_UNUSED(m);
    xtouch_led_apply();
}

void xtouch_led_setup()
{
    lv_msg_subscribe(XTOUCH_SETTINGS_LED_APPLY, (lv_msg_subscribe_cb_t)xtouch_led_onApply, NULL);
    xtouch_led_apply();
}

// Animated modes. Call from loop(); it self-throttles to ~30ms.
void xtouch_led_loop()
{
    if (!xTouchConfig.xTouchLedEnabled || !xtouch_led_ready)
        return;
    if (xTouchConfig.xTouchLedMode < 2)
        return; // solid/off need no animation

    unsigned long now = millis();
    if (now - xtouch_led_lastTick < 30)
        return;
    xtouch_led_lastTick = now;

    if (xTouchConfig.xTouchLedMode == 2) // breathe
    {
        xtouch_led_phase += 3;
        // triangle wave 40..255 for a smooth pulse
        uint8_t tri = xtouch_led_phase < 128 ? xtouch_led_phase * 2 : (255 - xtouch_led_phase) * 2;
        uint8_t s = 40 + (tri * (215) / 255);
        xtouch_led_fill(xtouch_led_scale(xTouchConfig.xTouchLedColor, s));
        xtouch_led_strip.show();
    }
    else if (xTouchConfig.xTouchLedMode == 3) // progress fill
    {
        uint32_t on = xtouch_led_statusColor();
        int lit = (int)xtouch_led_strip.numPixels() * bambuStatus.mc_print_percent / 100;
        uint8_t r = (on >> 16) & 0xFF, g = (on >> 8) & 0xFF, b = on & 0xFF;
        for (uint16_t i = 0; i < xtouch_led_strip.numPixels(); i++)
            xtouch_led_strip.setPixelColor(i, i < lit ? xtouch_led_strip.Color(r, g, b) : 0);
        xtouch_led_strip.show();
    }
    else if (xTouchConfig.xTouchLedMode == 4) // status-sync (solid, follows state)
    {
        xtouch_led_fill(xtouch_led_statusColor());
        xtouch_led_strip.show();
    }
}

#endif
