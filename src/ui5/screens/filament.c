#include "ui.h"

static void fil_cb(lv_event_t *e)
{
    sendMqttMsg((int)(intptr_t)lv_event_get_user_data(e), 0);
}

static void fil_button(lv_obj_t *grid, const char *sym, const char *title, const char *sub, int cmd)
{
    lv_obj_t *b = lv_btn_create(grid);
    ui5_style_card(b);
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

    static lv_coord_t cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t rows[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(content, cols, rows);
    lv_obj_set_layout(content, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(content, 12, 0);
    lv_obj_set_style_pad_column(content, 12, 0);

    fil_button(content, LV_SYMBOL_DOWNLOAD, "Load", "Feed new filament", XTOUCH_COMMAND_LOAD_FILAMENT);
    lv_obj_set_grid_cell(lv_obj_get_child(content, 0), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    fil_button(content, LV_SYMBOL_UPLOAD, "Unload", "Retract & remove", XTOUCH_COMMAND_UNLOAD_FILAMENT);
    lv_obj_set_grid_cell(lv_obj_get_child(content, 1), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    fil_button(content, LV_SYMBOL_DOWN, "Extrude", "Push filament", XTOUCH_COMMAND_EXTRUDE_UP);
    lv_obj_set_grid_cell(lv_obj_get_child(content, 2), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    fil_button(content, LV_SYMBOL_UP, "Retract", "Pull filament back", XTOUCH_COMMAND_EXTRUDE_DOWN);
    lv_obj_set_grid_cell(lv_obj_get_child(content, 3), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
}
