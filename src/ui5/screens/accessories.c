#include "ui.h"

// Accessories: a working "Lighting" panel for a user-attached WS2812/NeoPixel
// strip (data on a spare GPIO, default 18), plus a placeholder for the BIQU
// Panda integrations still to come.
//
// Controls mutate xTouchConfig, persist via xtouch_settings_save(), and send
// XTOUCH_SETTINGS_LED_APPLY so xtouch/led.h reapplies the strip.

void xtouch_settings_save(bool onlyRoot);

static const char *led_mode_names[5] = {"Off", "Solid", "Breathe", "Progress", "Status"};
static lv_obj_t *modeBtns[5];
static lv_obj_t *colorSw[7];
static const uint32_t colorVals[7] = {0xFFFFFF, 0xFF5D5D, 0xFFB020, 0x46D19E, 0x35D0BA, 0x4C8DFF, 0xA78BFA};

static void led_changed(void)
{
    xtouch_settings_save(false);
    sendMqttMsg(XTOUCH_SETTINGS_LED_APPLY, 0);
}

static void enable_cb(lv_event_t *e)
{
    xTouchConfig.xTouchLedEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    led_changed();
}

static void restyle_modes(void)
{
    for (int i = 0; i < 5; i++)
        lv_obj_set_style_bg_color(modeBtns[i], i == xTouchConfig.xTouchLedMode ? ui_theme_accent() : ui_theme_surface2(), 0);
}
static void mode_cb(lv_event_t *e)
{
    xTouchConfig.xTouchLedMode = (uint8_t)(intptr_t)lv_event_get_user_data(e);
    restyle_modes();
    led_changed();
}

static void restyle_colors(void)
{
    for (int i = 0; i < 7; i++)
    {
        bool on = (colorVals[i] == xTouchConfig.xTouchLedColor);
        lv_obj_set_style_border_color(colorSw[i], ui_theme_text(), 0);
        lv_obj_set_style_border_width(colorSw[i], on ? 2 : 0, 0);
    }
}
static void color_cb(lv_event_t *e)
{
    xTouchConfig.xTouchLedColor = (uint32_t)(intptr_t)lv_event_get_user_data(e);
    restyle_colors();
    led_changed();
}

static void brightness_cb(lv_event_t *e)
{
    xTouchConfig.xTouchLedBrightness = lv_slider_get_value(lv_event_get_target(e));
    led_changed();
}

static lv_obj_t *sub_label(lv_obj_t *parent, const char *txt)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
    return l;
}

static void build_lighting(lv_obj_t *content)
{
    lv_obj_t *card = lv_obj_create(content);
    ui5_style_card(card);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(card, 16, 0);
    lv_obj_set_style_pad_row(card, 12, 0);

    // header + enable
    lv_obj_t *head = lv_obj_create(card);
    lv_obj_remove_style_all(head);
    lv_obj_set_width(head, LV_PCT(100));
    lv_obj_set_height(head, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *title = lv_label_create(head);
    lv_label_set_text(title, LV_SYMBOL_CHARGE "  Lighting (WS2812)");
    lv_obj_set_style_text_color(title, ui_theme_text(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_t *en = lv_switch_create(head);
    lv_obj_set_style_bg_color(en, ui_theme_accent(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (xTouchConfig.xTouchLedEnabled)
        lv_obj_add_state(en, LV_STATE_CHECKED);
    lv_obj_add_event_cb(en, enable_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // mode segmented
    sub_label(card, "Effect");
    lv_obj_t *modeRow = lv_obj_create(card);
    lv_obj_remove_style_all(modeRow);
    lv_obj_set_width(modeRow, LV_PCT(100));
    lv_obj_set_height(modeRow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(modeRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(modeRow, 8, 0);
    for (int i = 0; i < 5; i++)
    {
        modeBtns[i] = lv_btn_create(modeRow);
        lv_obj_set_flex_grow(modeBtns[i], 1);
        lv_obj_add_event_cb(modeBtns[i], mode_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        lv_obj_t *l = lv_label_create(modeBtns[i]);
        lv_label_set_text(l, led_mode_names[i]);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
        lv_obj_center(l);
    }
    restyle_modes();

    // brightness
    sub_label(card, "Brightness");
    lv_obj_t *br = lv_slider_create(card);
    lv_obj_set_width(br, LV_PCT(100));
    lv_slider_set_range(br, 5, 255);
    lv_slider_set_value(br, xTouchConfig.xTouchLedBrightness, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(br, ui_theme_accent(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(br, ui_theme_accent(), LV_PART_KNOB);
    lv_obj_add_event_cb(br, brightness_cb, LV_EVENT_RELEASED, NULL);

    // color swatches
    sub_label(card, "Color (Solid / Breathe)");
    lv_obj_t *colRow = lv_obj_create(card);
    lv_obj_remove_style_all(colRow);
    lv_obj_set_width(colRow, LV_PCT(100));
    lv_obj_set_height(colRow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(colRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(colRow, 10, 0);
    for (int i = 0; i < 7; i++)
    {
        colorSw[i] = lv_obj_create(colRow);
        lv_obj_remove_style_all(colorSw[i]);
        lv_obj_set_size(colorSw[i], 34, 34);
        lv_obj_set_style_radius(colorSw[i], 9, 0);
        lv_obj_set_style_bg_color(colorSw[i], lv_color_hex(colorVals[i]), 0);
        lv_obj_set_style_bg_opa(colorSw[i], LV_OPA_COVER, 0);
        lv_obj_add_flag(colorSw[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(colorSw[i], color_cb, LV_EVENT_CLICKED, (void *)(intptr_t)colorVals[i]);
    }
    restyle_colors();

    // wiring hint
    char hint[64];
    snprintf(hint, sizeof hint, "Data on GPIO %d  ·  %d LEDs  ·  external 5V + common GND",
             xTouchConfig.xTouchLedGpio, xTouchConfig.xTouchLedCount);
    sub_label(card, hint);
}

static void build_panda(lv_obj_t *content)
{
    lv_obj_t *c = lv_obj_create(content);
    ui5_style_card(c);
    lv_obj_set_width(c, LV_PCT(100));
    lv_obj_set_height(c, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(c, 16, 0);
    lv_obj_set_style_pad_column(c, 14, 0);

    lv_obj_t *ic = lv_label_create(c);
    lv_label_set_text(ic, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(ic, ui_theme_accent(), 0);
    lv_obj_set_style_text_font(ic, &lv_font_montserrat_28, 0);

    lv_obj_t *col = lv_obj_create(c);
    lv_obj_remove_style_all(col);
    lv_obj_set_flex_grow(col, 1);
    lv_obj_set_height(col, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 3, 0);
    lv_obj_t *t = lv_label_create(col);
    lv_label_set_text(t, "BIQU Panda");
    lv_obj_set_style_text_color(t, ui_theme_text(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_20, 0);
    lv_obj_t *b = lv_label_create(col);
    lv_label_set_long_mode(b, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(b, LV_PCT(100));
    lv_label_set_text(b, "Breath chamber heater over MQTT; Status / Lux lights over their local web UI.");
    lv_obj_set_style_text_color(b, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(b, &lv_font_montserrat_14, 0);

    lv_obj_t *soon = lv_label_create(c);
    lv_label_set_text(soon, "SOON");
    lv_obj_set_style_text_color(soon, ui_theme_accent(), 0);
    lv_obj_set_style_text_font(soon, &lv_font_montserrat_14, 0);
}

void ui_accessoriesScreen_screen_init(void)
{
    ui_accessoriesScreen = lv_obj_create(NULL);
    lv_obj_t *content = ui5_build_screen(ui_accessoriesScreen, UI5_SCREEN_ACCESSORIES, "Accessories");
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(content, 12, 0);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    build_lighting(content);
    build_panda(content);
}
