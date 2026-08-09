#include "../ui.h"

// Jog pad. Each direction sets the active axis on the shared controlMode global
// (read by the device command layer) and then sends the move command. Axis/step
// mapping should be confirmed on hardware.
static lv_obj_t *lbl1mm, *lbl10mm;

static void jog_cb(lv_event_t *e)
{
    int cmd = (int)(intptr_t)lv_event_get_user_data(e);
    if (cmd == XTOUCH_COMMAND_HOME)
    {
        sendMqttMsg(XTOUCH_COMMAND_HOME, 0);
        return;
    }
    // Z uses the up/down commands with the Z axis selected; XY uses XY axis.
    controlMode.axis = (cmd == 1000 || cmd == 1001) ? ControlAxisZ : ControlAxisXY;
    if (cmd == 1000)
        sendMqttMsg(XTOUCH_COMMAND_UP, 0);
    else if (cmd == 1001)
        sendMqttMsg(XTOUCH_COMMAND_DOWN, 0);
    else
        sendMqttMsg(cmd, 0);
}

static void step_cb(lv_event_t *e)
{
    controlMode.inc = (int)(intptr_t)lv_event_get_user_data(e);
    lv_obj_set_style_bg_color(lbl1mm, controlMode.inc == 1 ? ui_theme_accent() : ui_theme_surface2(), 0);
    lv_obj_set_style_bg_color(lbl10mm, controlMode.inc == 10 ? ui_theme_accent() : ui_theme_surface2(), 0);
    sendMqttMsg(XTOUCH_CONTROL_INC_SWITCH, controlMode.inc);
}

static lv_obj_t *keycap(lv_obj_t *parent, const char *sym, int cmd, bool accent)
{
    lv_obj_t *b = lv_btn_create(parent);
    lv_obj_set_size(b, 78, 78);
    lv_obj_set_style_bg_color(b, accent ? ui_theme_accent() : ui_theme_surface(), 0);
    lv_obj_set_style_border_color(b, ui_theme_line(), 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_add_event_cb(b, jog_cb, LV_EVENT_CLICKED, (void *)(intptr_t)cmd);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, sym);
    lv_obj_set_style_text_color(l, accent ? ui_theme_on_accent() : ui_theme_text(), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_20, 0);
    lv_obj_center(l);
    return b;
}

void ui_controlScreen_screen_init(void)
{
    ui_controlScreen = lv_obj_create(NULL);
    lv_obj_t *content = ui5_build_screen(ui_controlScreen, UI5_SCREEN_CONTROL, "Control");
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(content, 16, 0);

    // XY jog pad (3x3)
    lv_obj_t *pad = lv_obj_create(content);
    lv_obj_remove_style_all(pad);
    lv_obj_set_size(pad, 254, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(pad, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pad, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_width(pad, 254);
    lv_obj_set_style_pad_all(pad, 0, 0);
    lv_obj_set_style_pad_row(pad, 8, 0);
    lv_obj_set_style_pad_column(pad, 8, 0);
    lv_obj_set_flex_flow(pad, LV_FLEX_FLOW_ROW_WRAP);

    // row 1
    lv_obj_t *sp1 = lv_obj_create(pad); lv_obj_remove_style_all(sp1); lv_obj_set_size(sp1, 78, 78);
    keycap(pad, LV_SYMBOL_UP, XTOUCH_COMMAND_UP, false);
    lv_obj_t *sp2 = lv_obj_create(pad); lv_obj_remove_style_all(sp2); lv_obj_set_size(sp2, 78, 78);
    // row 2
    keycap(pad, LV_SYMBOL_LEFT, XTOUCH_COMMAND_LEFT, false);
    keycap(pad, LV_SYMBOL_HOME, XTOUCH_COMMAND_HOME, true);
    keycap(pad, LV_SYMBOL_RIGHT, XTOUCH_COMMAND_RIGHT, false);
    // row 3
    lv_obj_t *sp3 = lv_obj_create(pad); lv_obj_remove_style_all(sp3); lv_obj_set_size(sp3, 78, 78);
    keycap(pad, LV_SYMBOL_DOWN, XTOUCH_COMMAND_DOWN, false);
    lv_obj_t *sp4 = lv_obj_create(pad); lv_obj_remove_style_all(sp4); lv_obj_set_size(sp4, 78, 78);

    // right column: Z + step
    lv_obj_t *side = lv_obj_create(content);
    lv_obj_remove_style_all(side);
    lv_obj_set_flex_grow(side, 1);
    lv_obj_set_height(side, LV_PCT(100));
    lv_obj_set_flex_flow(side, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(side, 10, 0);

    lv_obj_t *zt = lv_label_create(side);
    lv_label_set_text(zt, "Z axis");
    lv_obj_set_style_text_color(zt, ui_theme_muted(), 0);

    lv_obj_t *zrow = lv_obj_create(side);
    lv_obj_remove_style_all(zrow);
    lv_obj_set_width(zrow, LV_PCT(100));
    lv_obj_set_height(zrow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(zrow, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(zrow, 10, 0);
    keycap(zrow, LV_SYMBOL_UP " Z", 1000, false);
    keycap(zrow, LV_SYMBOL_DOWN " Z", 1001, false);

    lv_obj_t *st = lv_label_create(side);
    lv_label_set_text(st, "Step");
    lv_obj_set_style_text_color(st, ui_theme_muted(), 0);

    lv_obj_t *steprow = lv_obj_create(side);
    lv_obj_remove_style_all(steprow);
    lv_obj_set_width(steprow, LV_PCT(100));
    lv_obj_set_height(steprow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(steprow, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(steprow, 10, 0);

    lbl1mm = lv_btn_create(steprow);
    lv_obj_set_flex_grow(lbl1mm, 1);
    lv_obj_set_style_bg_color(lbl1mm, controlMode.inc == 1 ? ui_theme_accent() : ui_theme_surface2(), 0);
    lv_obj_add_event_cb(lbl1mm, step_cb, LV_EVENT_CLICKED, (void *)(intptr_t)1);
    lv_obj_t *l1 = lv_label_create(lbl1mm);
    lv_label_set_text(l1, "1 mm");
    lv_obj_center(l1);

    lbl10mm = lv_btn_create(steprow);
    lv_obj_set_flex_grow(lbl10mm, 1);
    lv_obj_set_style_bg_color(lbl10mm, controlMode.inc == 10 ? ui_theme_accent() : ui_theme_surface2(), 0);
    lv_obj_add_event_cb(lbl10mm, step_cb, LV_EVENT_CLICKED, (void *)(intptr_t)10);
    lv_obj_t *l10 = lv_label_create(lbl10mm);
    lv_label_set_text(l10, "10 mm");
    lv_obj_center(l10);
}
