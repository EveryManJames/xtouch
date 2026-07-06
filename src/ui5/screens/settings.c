#include "ui.h"

// Defined in settings.h (main TU); forward-declared for the simple flag toggles
// that don't need a hardware side effect.
void xtouch_settings_save(bool onlyRoot);

// ---------- theme controls ----------
static void accent_cb(lv_event_t *e)
{
    ui_theme_set_accent((uint8_t)(intptr_t)lv_event_get_user_data(e));
}
static void mode_cb(lv_event_t *e)
{
    ui_theme_set_mode((uint8_t)(intptr_t)lv_event_get_user_data(e));
}

// ---------- display controls (handled by xtouch/events.h via the message bus) ----------
static void backlight_changed_cb(lv_event_t *e) { LV_UNUSED(e); sendMqttMsg(XTOUCH_SETTINGS_BACKLIGHT, 0); }
static void backlight_released_cb(lv_event_t *e) { LV_UNUSED(e); sendMqttMsg(XTOUCH_SETTINGS_BACKLIGHT_SET, 0); }
static void tftoff_released_cb(lv_event_t *e) { LV_UNUSED(e); sendMqttMsg(XTOUCH_SETTINGS_TFTOFF_SET, 0); }
static void invert_cb(lv_event_t *e) { LV_UNUSED(e); sendMqttMsg(XTOUCH_SETTINGS_TFT_INVERT, 0); }
static void flip_cb(lv_event_t *e) { LV_UNUSED(e); sendMqttMsg(XTOUCH_SETTINGS_TFT_FLIP, 0); }

// ---------- xtouch flag toggles ----------
static void chamber_sensor_cb(lv_event_t *e)
{
    xTouchConfig.xTouchChamberSensorEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    xtouch_settings_save(false);
    sendMqttMsg(XTOUCH_SETTINGS_CHAMBER_TEMP, 0); // events.h starts/stops the sensor timer
}
static void auxfan_cb(lv_event_t *e)
{
    xTouchConfig.xTouchAuxFanEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    xtouch_settings_save(false);
}
static void chamberfan_cb(lv_event_t *e)
{
    xTouchConfig.xTouchChamberFanEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    xtouch_settings_save(false);
}
static void ota_cb(lv_event_t *e)
{
    xTouchConfig.xTouchOTAEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    xtouch_settings_save(false);
}
static void wop_cb(lv_event_t *e)
{
    xTouchConfig.xTouchWakeOnPrint = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    xtouch_settings_save(false);
}

// ---------- small builders ----------
static lv_obj_t *group(lv_obj_t *parent, const char *title)
{
    lv_obj_t *g = lv_obj_create(parent);
    lv_obj_remove_style_all(g);
    lv_obj_set_width(g, LV_PCT(100));
    lv_obj_set_height(g, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(g, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(g, 8, 0);
    lv_obj_t *t = lv_label_create(g);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, ui_theme_accent(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_14, 0);
    return g;
}

static lv_obj_t *row(lv_obj_t *parent, const char *name)
{
    lv_obj_t *r = lv_obj_create(parent);
    ui5_style_card(r);
    lv_obj_set_width(r, LV_PCT(100));
    lv_obj_set_height(r, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(r, 12, 0);
    lv_obj_t *l = lv_label_create(r);
    lv_label_set_text(l, name);
    lv_obj_set_style_text_color(l, ui_theme_text(), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
    return r;
}

static lv_obj_t *switch_row(lv_obj_t *parent, const char *name, bool checked, lv_event_cb_t cb)
{
    lv_obj_t *r = row(parent, name);
    lv_obj_t *sw = lv_switch_create(r);
    lv_obj_set_style_bg_color(sw, ui_theme_accent(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (checked)
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, cb, LV_EVENT_VALUE_CHANGED, NULL);
    return sw;
}

void ui_settingsScreen_screen_init(void)
{
    ui_settingsScreen = lv_obj_create(NULL);
    lv_obj_t *content = ui5_build_screen(ui_settingsScreen, UI5_SCREEN_SETTINGS, "Settings");
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(content, 12, 0);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    // ---- Theme ----
    lv_obj_t *themeG = group(content, "Theme");
    lv_obj_t *themeCard = lv_obj_create(themeG);
    ui5_style_card(themeCard);
    lv_obj_set_width(themeCard, LV_PCT(100));
    lv_obj_set_height(themeCard, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(themeCard, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(themeCard, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(themeCard, 12, 0);
    lv_obj_set_style_pad_column(themeCard, 10, 0);

    // dark / light segmented
    for (int m = 1; m >= 0; m--) // 1=Dark, 0=Light
    {
        bool active = (ui_theme_is_dark() == (m == 1));
        lv_obj_t *seg = lv_btn_create(themeCard);
        lv_obj_set_style_bg_color(seg, active ? ui_theme_accent() : ui_theme_surface2(), 0);
        lv_obj_add_event_cb(seg, mode_cb, LV_EVENT_CLICKED, (void *)(intptr_t)m);
        lv_obj_t *sl = lv_label_create(seg);
        lv_label_set_text(sl, m == 1 ? "Dark" : "Light");
        lv_obj_set_style_text_color(sl, active ? ui_theme_on_accent() : ui_theme_muted(), 0);
        lv_obj_center(sl);
    }

    lv_obj_t *swSpacer = lv_obj_create(themeCard);
    lv_obj_remove_style_all(swSpacer);
    lv_obj_set_height(swSpacer, 1);
    lv_obj_set_flex_grow(swSpacer, 1);

    for (uint8_t i = 0; i < UI_THEME_ACCENT_COUNT; i++)
    {
        lv_obj_t *sw = lv_obj_create(themeCard);
        lv_obj_remove_style_all(sw);
        lv_obj_set_size(sw, 34, 34);
        lv_obj_set_style_radius(sw, 9, 0);
        lv_obj_set_style_bg_color(sw, ui_theme_accent_at(i), 0);
        lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, 0);
        if (i == ui_theme_accent_index())
        {
            lv_obj_set_style_border_color(sw, ui_theme_text(), 0);
            lv_obj_set_style_border_width(sw, 2, 0);
        }
        lv_obj_add_flag(sw, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(sw, accent_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }

    // ---- Display ----
    lv_obj_t *dispG = group(content, "Display");

    lv_obj_t *blRow = row(dispG, "Backlight");
    ui_settingsBackLightPanelSlider = lv_slider_create(blRow);
    lv_slider_set_range(ui_settingsBackLightPanelSlider, 10, 255);
    lv_slider_set_value(ui_settingsBackLightPanelSlider, xTouchConfig.xTouchBacklightLevel, LV_ANIM_OFF);
    lv_obj_set_width(ui_settingsBackLightPanelSlider, 220);
    lv_obj_set_style_bg_color(ui_settingsBackLightPanelSlider, ui_theme_accent(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ui_settingsBackLightPanelSlider, ui_theme_accent(), LV_PART_KNOB);
    lv_obj_add_event_cb(ui_settingsBackLightPanelSlider, backlight_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingsBackLightPanelSlider, backlight_released_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_t *toRow = row(dispG, "Sleep after (min)");
    ui_settingsTFTOFFSlider = lv_slider_create(toRow);
    lv_slider_set_range(ui_settingsTFTOFFSlider, 0, 60);
    lv_slider_set_value(ui_settingsTFTOFFSlider, xTouchConfig.xTouchTFTOFFValue, LV_ANIM_OFF);
    lv_obj_set_width(ui_settingsTFTOFFSlider, 220);
    lv_obj_set_style_bg_color(ui_settingsTFTOFFSlider, ui_theme_accent(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ui_settingsTFTOFFSlider, ui_theme_accent(), LV_PART_KNOB);
    lv_obj_add_event_cb(ui_settingsTFTOFFSlider, tftoff_released_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_t *invRow = row(dispG, "Invert colors");
    ui_settingsTFTInvertSwitch = lv_switch_create(invRow);
    lv_obj_set_style_bg_color(ui_settingsTFTInvertSwitch, ui_theme_accent(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (xTouchConfig.xTouchTFTInvert)
        lv_obj_add_state(ui_settingsTFTInvertSwitch, LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui_settingsTFTInvertSwitch, invert_cb, LV_EVENT_VALUE_CHANGED, NULL);

    switch_row(dispG, "Flip screen", xTouchConfig.xTouchTFTFlip, flip_cb);

    // ---- xtouch ----
    lv_obj_t *xG = group(content, "xtouch");
    switch_row(xG, "Aux fan", xTouchConfig.xTouchAuxFanEnabled, auxfan_cb);
    switch_row(xG, "Chamber sensor", xTouchConfig.xTouchChamberSensorEnabled, chamber_sensor_cb);
    switch_row(xG, "Chamber fan", xTouchConfig.xTouchChamberFanEnabled, chamberfan_cb);
    switch_row(xG, "Wake on print", xTouchConfig.xTouchWakeOnPrint, wop_cb);
    switch_row(xG, "OTA updates", xTouchConfig.xTouchOTAEnabled, ota_cb);
}
