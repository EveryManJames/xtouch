#include "ui.h"
#include "ui_theme.h"
#include "xtouch/types.h"

// Defined in settings.h (single TU / main.cpp). Forward-declared here so this
// separate translation unit can persist a theme change without pulling in the
// header-only settings definitions.
void xtouch_settings_save(bool onlyRoot);

// ---- accent presets (index order must match the Settings swatches) ----
static const uint32_t ui_theme_accents[UI_THEME_ACCENT_COUNT] = {
    0x35D0BA, // 0 Teal (default)
    0xD97757, // 1 Claude Orange
    0x4C8DFF, // 2 Blue
    0xA78BFA, // 3 Violet
    0xFF6F9C, // 4 Rose
    0xFFB020, // 5 Amber
    0x35C46B, // 6 Green
};
static const char *ui_theme_accent_names[UI_THEME_ACCENT_COUNT] = {
    "Teal", "Claude", "Blue", "Violet", "Rose", "Amber", "Green"};

static uint8_t ui_theme_clamp_accent(uint8_t idx)
{
    return idx < UI_THEME_ACCENT_COUNT ? idx : 0;
}

uint8_t ui_theme_accent_index(void) { return ui_theme_clamp_accent(xTouchConfig.xTouchThemeAccent); }
uint8_t ui_theme_is_dark(void) { return xTouchConfig.xTouchThemeDark ? 1 : 0; }

void ui_theme_init(void)
{
    xTouchConfig.xTouchThemeAccent = ui_theme_clamp_accent(xTouchConfig.xTouchThemeAccent);
}

// Deferred so we never delete the active screen from inside its own event
// (e.g. the swatch that was just tapped). Runs on the next lv_timer_handler.
static void ui_theme_async_reload(void *unused)
{
    LV_UNUSED(unused);
    loadScreen(xTouchConfig.currentScreenIndex);
}

void ui_theme_set_accent(uint8_t idx)
{
    xTouchConfig.xTouchThemeAccent = ui_theme_clamp_accent(idx);
    xtouch_settings_save(false);
    lv_async_call(ui_theme_async_reload, NULL);
}

void ui_theme_set_mode(uint8_t dark)
{
    xTouchConfig.xTouchThemeDark = dark ? true : false;
    xtouch_settings_save(false);
    lv_async_call(ui_theme_async_reload, NULL);
}

// ---- neutral ground ----
lv_color_t ui_theme_bg(void) { return lv_color_hex(ui_theme_is_dark() ? 0x0D1014 : 0xEEF1F5); }
lv_color_t ui_theme_surface(void) { return lv_color_hex(ui_theme_is_dark() ? 0x161B22 : 0xFFFFFF); }
lv_color_t ui_theme_surface2(void) { return lv_color_hex(ui_theme_is_dark() ? 0x1F2630 : 0xF2F5F9); }
lv_color_t ui_theme_surface3(void) { return lv_color_hex(ui_theme_is_dark() ? 0x262F3A : 0xE6EBF1); }
lv_color_t ui_theme_line(void) { return lv_color_hex(ui_theme_is_dark() ? 0x2A323D : 0xD9DFE7); }
lv_color_t ui_theme_text(void) { return lv_color_hex(ui_theme_is_dark() ? 0xE6EDF3 : 0x10151C); }
lv_color_t ui_theme_muted(void) { return lv_color_hex(ui_theme_is_dark() ? 0x8A94A3 : 0x5C6672); }
lv_color_t ui_theme_dim(void) { return lv_color_hex(ui_theme_is_dark() ? 0x5B6472 : 0x9AA4B0); }

// ---- accent ----
lv_color_t ui_theme_accent_at(uint8_t idx) { return lv_color_hex(ui_theme_accents[ui_theme_clamp_accent(idx)]); }
const char *ui_theme_accent_name(uint8_t idx) { return ui_theme_accent_names[ui_theme_clamp_accent(idx)]; }
lv_color_t ui_theme_accent(void) { return ui_theme_accent_at(ui_theme_accent_index()); }

// Pick black or white for text/icons drawn on top of the accent, by luminance.
lv_color_t ui_theme_on_accent(void)
{
    uint32_t c = ui_theme_accents[ui_theme_accent_index()];
    uint32_t r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
    uint32_t lum = (r * 299 + g * 587 + b * 114) / 1000;
    return lum > 150 ? lv_color_hex(0x0C1116) : lv_color_hex(0xFFFFFF);
}

// ---- semantic (fixed) ----
lv_color_t ui_theme_heat(void) { return lv_color_hex(0xFF7849); }
lv_color_t ui_theme_chamber(void) { return lv_color_hex(0x58B6FF); }
lv_color_t ui_theme_running(void) { return lv_color_hex(0x46D19E); }
lv_color_t ui_theme_pause(void) { return lv_color_hex(0xFFC24B); }
lv_color_t ui_theme_stop(void) { return lv_color_hex(0xFF5D5D); }
