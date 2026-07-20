#include "ui.h"

// Shared 800x480 chrome: a left icon nav rail + a top status strip, themed from
// the active palette. Rebuilt per screen load (each screen root is deleted and
// recreated by loadScreen), so active state is set inline at build time.

static const char *ui5_nav_symbols[UI5_SCREEN_COUNT] = {
    LV_SYMBOL_HOME,     // Home
    LV_SYMBOL_TINT,     // Temperature
    LV_SYMBOL_GPS,      // Control / move
    LV_SYMBOL_DOWNLOAD, // Filament
    LV_SYMBOL_SETTINGS, // Settings
    LV_SYMBOL_CHARGE,   // Accessories / lighting
};

void ui5_apply_screen_bg(lv_obj_t *root)
{
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(root, ui_theme_bg(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_pad_row(root, 0, 0);
    lv_obj_set_style_pad_column(root, 0, 0);
}

void ui5_style_card(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, ui_theme_surface(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, ui_theme_line(), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 14, 0);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

static void ui5_nav_click_cb(lv_event_t *e)
{
    int target = (int)(intptr_t)lv_event_get_user_data(e);
    loadScreen(target);
}

static void ui5_build_nav(lv_obj_t *root, int active)
{
    lv_obj_t *nav = lv_obj_create(root);
    lv_obj_remove_style_all(nav);
    lv_obj_set_size(nav, 72, 480);
    lv_obj_set_style_bg_color(nav, ui_theme_surface(), 0);
    lv_obj_set_style_bg_opa(nav, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(nav, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_color(nav, ui_theme_line(), 0);
    lv_obj_set_style_border_width(nav, 1, 0);
    lv_obj_clear_flag(nav, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(nav, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(nav, 10, 0);
    lv_obj_set_style_pad_row(nav, 8, 0);

    // logo tile
    lv_obj_t *logo = lv_obj_create(nav);
    lv_obj_remove_style_all(logo);
    lv_obj_set_size(logo, 36, 36);
    lv_obj_set_style_radius(logo, 10, 0);
    lv_obj_set_style_bg_color(logo, ui_theme_accent(), 0);
    lv_obj_set_style_bg_opa(logo, LV_OPA_COVER, 0);
    lv_obj_set_style_margin_bottom(logo, 6, 0);
    lv_obj_t *logoTxt = lv_label_create(logo);
    lv_label_set_text(logoTxt, "x");
    lv_obj_set_style_text_color(logoTxt, ui_theme_on_accent(), 0);
    lv_obj_set_style_text_font(logoTxt, &lv_font_montserrat_20, 0);
    lv_obj_center(logoTxt);

    for (int i = 0; i < UI5_SCREEN_COUNT; i++)
    {
        bool on = (i == active);
        lv_obj_t *b = lv_obj_create(nav);
        lv_obj_remove_style_all(b);
        lv_obj_set_size(b, 50, 50);
        lv_obj_set_style_radius(b, 12, 0);
        lv_obj_set_style_bg_color(b, on ? ui_theme_accent() : ui_theme_surface2(), 0);
        lv_obj_set_style_bg_opa(b, on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(b, ui5_nav_click_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        lv_obj_t *ic = lv_label_create(b);
        lv_label_set_text(ic, ui5_nav_symbols[i]);
        lv_obj_set_style_text_font(ic, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(ic, on ? ui_theme_on_accent() : ui_theme_muted(), 0);
        lv_obj_center(ic);
    }
}

static void ui5_build_topbar(lv_obj_t *col, const char *title)
{
    lv_obj_t *top = lv_obj_create(col);
    lv_obj_remove_style_all(top);
    lv_obj_set_size(top, LV_PCT(100), 46);
    lv_obj_set_style_bg_color(top, ui_theme_surface(), 0);
    lv_obj_set_style_bg_opa(top, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(top, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(top, ui_theme_line(), 0);
    lv_obj_set_style_border_width(top, 1, 0);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(top, 16, 0);
    lv_obj_set_style_pad_right(top, 16, 0);
    lv_obj_set_style_pad_column(top, 12, 0);

    lv_obj_t *t = lv_label_create(top);
    lv_label_set_text(t, title ? title : "");
    lv_obj_set_style_text_color(t, ui_theme_text(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_16, 0);

    lv_obj_t *spacer = lv_obj_create(top);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_height(spacer, 1);
    lv_obj_set_flex_grow(spacer, 1);

    const char *name = (xTouchConfig.xTouchPrinterName[0] != '\0') ? xTouchConfig.xTouchPrinterName
                       : (xTouchConfig.xTouchPrinterModel[0] != '\0') ? xTouchConfig.xTouchPrinterModel
                                                                      : "xtouch";
    lv_obj_t *pn = lv_label_create(top);
    lv_label_set_text(pn, name);
    lv_obj_set_style_text_color(pn, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(pn, &lv_font_montserrat_14, 0);

    lv_obj_t *wifi = lv_label_create(top);
    lv_label_set_text(wifi, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi, ui_theme_accent(), 0);
    lv_obj_set_style_text_font(wifi, &lv_font_montserrat_16, 0);
}

lv_obj_t *ui5_build_screen(lv_obj_t *root, int index, const char *title)
{
    ui5_apply_screen_bg(root);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    ui5_build_nav(root, index);

    lv_obj_t *col = lv_obj_create(root);
    lv_obj_remove_style_all(col);
    lv_obj_set_flex_grow(col, 1);
    lv_obj_set_height(col, LV_PCT(100));
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);

    ui5_build_topbar(col, title);

    lv_obj_t *content = lv_obj_create(col);
    lv_obj_remove_style_all(content);
    lv_obj_set_width(content, LV_PCT(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(content, 16, 0);
    return content;
}
