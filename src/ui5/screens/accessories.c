#include "ui.h"

// Placeholder home for accessory integrations that land next:
//   * Lighting  - bring-your-own WS2812/NeoPixel strip on a spare GPIO, with
//                 status-reactive effects.
//   * BIQU Panda - Breath (chamber heater) over MQTT, Status/Lux lights over
//                 their local web UIs.
static void card(lv_obj_t *parent, const char *sym, const char *title, const char *body)
{
    lv_obj_t *c = lv_obj_create(parent);
    ui5_style_card(c);
    lv_obj_set_width(c, LV_PCT(100));
    lv_obj_set_height(c, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(c, 16, 0);
    lv_obj_set_style_pad_column(c, 14, 0);

    lv_obj_t *ic = lv_label_create(c);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, ui_theme_accent(), 0);
    lv_obj_set_style_text_font(ic, &lv_font_montserrat_28, 0);

    lv_obj_t *col = lv_obj_create(c);
    lv_obj_remove_style_all(col);
    lv_obj_set_flex_grow(col, 1);
    lv_obj_set_height(col, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 3, 0);

    lv_obj_t *t = lv_label_create(col);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, ui_theme_text(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_20, 0);

    lv_obj_t *b = lv_label_create(col);
    lv_label_set_long_mode(b, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(b, LV_PCT(100));
    lv_label_set_text(b, body);
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

    card(content, LV_SYMBOL_CHARGE, "Lighting (WS2812)",
         "Wire an addressable RGB strip to a spare GPIO and mirror print status, set colors and effects.");
    card(content, LV_SYMBOL_SETTINGS, "BIQU Panda",
         "Breath chamber heater over MQTT; Status / Lux lights over their local web UI.");
}
