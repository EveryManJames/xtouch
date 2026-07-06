#ifndef _XTOUCH_UI5_THEME
#define _XTOUCH_UI5_THEME

// Theme engine for the 5" (800x480) UI.
//
// A theme = one of UI_THEME_ACCENT_COUNT accent presets (the accent recolors
// everything interactive/active) plus a dark/light appearance for the neutral
// ground. Semantic colors (heat / chamber / running / paused / stop) are FIXED
// across every theme so status always reads the same.
//
// The selection lives in xTouchConfig (xTouchThemeAccent / xTouchThemeDark) and
// is persisted through xtouch_settings_save(), so it survives reboots.

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

#define UI_THEME_ACCENT_COUNT 7

    void ui_theme_init(void);

    uint8_t ui_theme_accent_index(void);
    uint8_t ui_theme_is_dark(void);

    // Persist + re-render the current screen with the new selection.
    void ui_theme_set_accent(uint8_t idx);
    void ui_theme_set_mode(uint8_t dark);

    // Neutral ground (appearance-dependent)
    lv_color_t ui_theme_bg(void);
    lv_color_t ui_theme_surface(void);
    lv_color_t ui_theme_surface2(void);
    lv_color_t ui_theme_surface3(void);
    lv_color_t ui_theme_line(void);
    lv_color_t ui_theme_text(void);
    lv_color_t ui_theme_muted(void);
    lv_color_t ui_theme_dim(void);

    // Accent (selection-dependent)
    lv_color_t ui_theme_accent(void);
    lv_color_t ui_theme_on_accent(void); // legible text/icon color on the accent
    lv_color_t ui_theme_accent_at(uint8_t idx);
    const char *ui_theme_accent_name(uint8_t idx);

    // Semantic (fixed)
    lv_color_t ui_theme_heat(void);
    lv_color_t ui_theme_chamber(void);
    lv_color_t ui_theme_running(void);
    lv_color_t ui_theme_pause(void);
    lv_color_t ui_theme_stop(void);

#ifdef __cplusplus
}
#endif

#endif
