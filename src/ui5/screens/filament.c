#include "../ui.h"

static void fil_cb(lv_event_t *e)
{
    sendMqttMsg((int)(intptr_t)lv_event_get_user_data(e), 0);
}

static void fil_button(lv_obj_t *parent, const char *sym, const char *title, const char *sub, int cmd)
{
    lv_obj_t *b = lv_btn_create(parent);
    ui5_style_card(b);
    lv_obj_set_flex_grow(b, 1);
    lv_obj_set_height(b, LV_PCT(100));
    lv_obj_set_flex_flow(b, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(b, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(b, 8, 0);
    lv_obj_add_event_cb(b, fil_cb, LV_EVENT_CLICKED, (void *)(intptr_t)cmd);

    lv_obj_t *ic = lv_label_create(b);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, ui_theme_accent(), 0);
    lv_obj_set_style_text_font(ic, &lv_font_montserrat_28, 0);

    lv_obj_t *t = lv_label_create(b);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, ui_theme_text(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_20, 0);

    lv_obj_t *s = lv_label_create(b);
    lv_label_set_text(s, sub);
    lv_obj_set_style_text_color(s, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(s, &lv_font_montserrat_14, 0);
}

void ui_filamentScreen_screen_init(void)
{
    ui_filamentScreen = lv_obj_create(NULL);
    lv_obj_t *content = ui5_build_screen(ui_filamentScreen, UI5_SCREEN_FILAMENT, "Filament");

    // 2x2 of buttons, built as two flex rows (LV_USE_GRID is off in lv_conf.h).
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(content, 12, 0);

    lv_obj_t *row1 = lv_obj_create(content);
    lv_obj_remove_style_all(row1);
    lv_obj_set_width(row1, LV_PCT(100));
    lv_obj_set_flex_grow(row1, 1);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row1, 12, 0);

    lv_obj_t *row2 = lv_obj_create(content);
    lv_obj_remove_style_all(row2);
    lv_obj_set_width(row2, LV_PCT(100));
    lv_obj_set_flex_grow(row2, 1);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row2, 12, 0);

    fil_button(row1, LV_SYMBOL_DOWNLOAD, "Load", "Feed new filament", XTOUCH_COMMAND_LOAD_FILAMENT);
    fil_button(row1, LV_SYMBOL_UPLOAD, "Unload", "Retract & remove", XTOUCH_COMMAND_UNLOAD_FILAMENT);
    fil_button(row2, LV_SYMBOL_DOWN, "Extrude", "Push filament", XTOUCH_COMMAND_EXTRUDE_UP);
    fil_button(row2, LV_SYMBOL_UP, "Retract", "Pull filament back", XTOUCH_COMMAND_EXTRUDE_DOWN);
}
