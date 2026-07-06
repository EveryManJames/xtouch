#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#include <Arduino_GFX_Library.h>
#include "esp_heap_caps.h"

// ---------------------------------------------------------------------------
// Guition JC8048W550 - 5" 800x480 IPS, ST7262 24-bit RGB parallel panel.
// Pinout / timing from the canonical Arduino_GFX config for this board.
// ---------------------------------------------------------------------------

// Backlight (PWM via LEDC)
#define LCD_BACK_LIGHT_PIN 2
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_12_BIT 12
#define LEDC_BASE_FREQ 5000

// TF/SD card is on a dedicated SPI bus on this board (not the default pins).
#define SD_SCK 12
#define SD_MISO 13
#define SD_MOSI 11
#define SD_CS 10

static const uint16_t screenWidth = 800;
static const uint16_t screenHeight = 480;

// RGB data + control bus. First three args (CS/SCK/SDA) are unused on this
// panel and passed as GFX_NOT_DEFINED.
Arduino_ESP32RGBPanel *xtouch_rgbpanel = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45, 48, 47, 21, 14,       /* R0-R4 */
    5, 6, 7, 15, 16, 4,       /* G0-G5 */
    8, 3, 46, 9, 1            /* B0-B4 */
);

// width, hsync(pol,fp,pw,bp), height, vsync(pol,fp,pw,bp), pclk_active_neg,
// prefer_speed (16 MHz), auto_flush.
Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
    xtouch_rgbpanel,
    800 /* width */, 0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
    480 /* height */, 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *xtouch_lvbuf = NULL;

#include "ui5/ui.h"
#include "touch.h"
#include "xtouch/globals.h"

bool xtouch_screen_touchFromPowerOff = false;

void xtouch_screen_ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);
    ledcWrite(channel, duty);
}

void xtouch_screen_setBrightness(byte brightness)
{
    xtouch_screen_ledcAnalogWrite(LEDC_CHANNEL_0, brightness);
}

void xtouch_screen_wakeUp()
{
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
    xtouch_screen_touchFromPowerOff = false;
    loadScreen(0);
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
}

void xtouch_screen_onScreenOff(lv_timer_t *timer)
{
    if (xTouchConfig.xTouchTFTOFFValue < XTOUCH_LCD_MIN_SLEEP_TIME)
    {
        return;
    }

    ConsoleInfo.println("[XTouch][SCREEN] Screen Off");
    xtouch_screen_setBrightness(0);
    xtouch_screen_touchFromPowerOff = true;
}

void xtouch_screen_setupScreenTimer()
{
    xtouch_screen_onScreenOffTimer = lv_timer_create(xtouch_screen_onScreenOff, xTouchConfig.xTouchTFTOFFValue * 1000 * 60, NULL);
    lv_timer_pause(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_startScreenTimer()
{
    lv_timer_resume(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_setScreenTimer(uint32_t period)
{
    lv_timer_set_period(xtouch_screen_onScreenOffTimer, period);
}

// The ST7262 RGB panel has no hardware colour-inversion command, so the
// "invert screen colours" setting has no effect on this build.
void xtouch_screen_invertColors()
{
}

byte xtouch_screen_getTFTFlip()
{
    byte val = xtouch_eeprom_read(XTOUCH_EEPROM_POS_TFTFLIP);
    xTouchConfig.xTouchTFTFlip = val;
    return val;
}

void xtouch_screen_setTFTFlip(byte mode)
{
    xTouchConfig.xTouchTFTFlip = mode;
    xtouch_eeprom_write(XTOUCH_EEPROM_POS_TFTFLIP, mode);
}

// RGB panels cannot rotate in hardware; use LVGL software rotation (enabled via
// disp_drv.sw_rotate) to flip 180 degrees. Requires the display to be
// registered first, so this is safe to call after xtouch_screen_setup().
void xtouch_screen_setupTFTFlip()
{
    lv_disp_t *disp = lv_disp_get_default();
    if (disp == NULL)
        return;
    lv_disp_set_rotation(disp, xtouch_screen_getTFTFlip() ? LV_DISP_ROT_180 : LV_DISP_ROT_NONE);
}

void xtouch_screen_toggleTFTFlip()
{
    xtouch_screen_setTFTFlip(!xtouch_screen_getTFTFlip());
    delay(200);
    ESP.restart();
}

void xtouch_screen_dispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

void xtouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    x_touch_gt911.read();

    if (x_touch_gt911.isTouched && x_touch_gt911.touches > 0)
    {
        lv_timer_reset(xtouch_screen_onScreenOffTimer);

        // Don't pass the first touch after wake-from-sleep through to the UI.
        if (xtouch_screen_touchFromPowerOff)
        {
            xtouch_screen_wakeUp();
            data->state = LV_INDEV_STATE_REL;
            return;
        }

        ScreenPoint sp = getScreenCoords(x_touch_gt911.points[0].x, x_touch_gt911.points[0].y);
        data->state = LV_INDEV_STATE_PR;
        data->point.x = sp.x;
        data->point.y = sp.y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

// SD begin hook (see sdcard.h). This board needs an explicit SPI bus + pins.
SPIClass xtouch_sd_spi = SPIClass(HSPI);
bool xtouch_screen_sdBegin()
{
    xtouch_sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    return SD.begin(SD_CS, xtouch_sd_spi);
}

void xtouch_screen_setup()
{
    ConsoleInfo.println("[XTouch][SCREEN] Setup (JC8048W550)");

    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    lv_init();

    gfx->begin();
    gfx->fillScreen(BLACK);

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);
    xtouch_screen_setBrightness(255);

    // Bring the touch controller up early so the intro/SD-wait screens are
    // already interactive.
    x_touch_gt911.begin();
    x_touch_gt911.setRotation(ROTATION_NORMAL);

    // Partial render buffer: 40 lines. Prefer internal RAM, fall back to PSRAM.
    uint32_t bufPixels = (uint32_t)screenWidth * 40;
    xtouch_lvbuf = (lv_color_t *)heap_caps_malloc(bufPixels * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (xtouch_lvbuf == NULL)
        xtouch_lvbuf = (lv_color_t *)heap_caps_malloc(bufPixels * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    lv_disp_draw_buf_init(&draw_buf, xtouch_lvbuf, NULL, bufPixels);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = xtouch_screen_dispFlush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.sw_rotate = 1; // enable software 180 flip support
    lv_disp_drv_register(&disp_drv);

    /*Initialize the input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = xtouch_screen_touchRead;
    lv_indev_drv_register(&indev_drv);

    /*Initialize the graphics library */
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    xtouch_screen_setupTFTFlip();

    initTopLayer();
}

#endif
