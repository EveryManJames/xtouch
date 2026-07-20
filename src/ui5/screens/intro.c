#include "ui.h"

// Boot / status screen. The boot sequence (coldboot, connection, firmware, mqtt,
// sdcard) writes progress text into introScreenCaption, so this screen must
// exist before any of that runs (main.cpp shows it first thing).
void ui_introScreen_screen_init(void)
{
    introScreen = lv_obj_create(NULL);
    ui5_apply_screen_bg(introScreen);
    lv_obj_set_flex_flow(introScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(introScreen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(introScreen, 18, 0);

    lv_obj_t *badge = lv_obj_create(introScreen);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 96, 96);
    lv_obj_set_style_radius(badge, 24, 0);
    lv_obj_set_style_bg_color(badge, ui_theme_accent(), 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);

    introScreenIcon = lv_label_create(badge);
    lv_label_set_text(introScreenIcon, "x");
    lv_obj_set_style_text_color(introScreenIcon, ui_theme_on_accent(), 0);
    lv_obj_set_style_text_font(introScreenIcon, &lv_font_montserrat_36, 0);
    lv_obj_center(introScreenIcon);

    lv_obj_t *word = lv_label_create(introScreen);
    lv_label_set_text(word, "xtouch");
    lv_obj_set_style_text_color(word, ui_theme_text(), 0);
    lv_obj_set_style_text_font(word, &lv_font_montserrat_28, 0);

    introScreenCaption = lv_label_create(introScreen);
    lv_label_set_text(introScreenCaption, "Starting");
    lv_obj_set_style_text_color(introScreenCaption, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(introScreenCaption, &lv_font_montserrat_16, 0);
}
