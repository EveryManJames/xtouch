#include "ui.h"

// ---- live-bound widgets (reassigned each time the screen is rebuilt) ----
static lv_obj_t *lblNozzle, *lblBed, *lblChamber, *swLight;
static lv_obj_t *readyPanel, *printPanel, *arcProgress, *lblPct, *lblLayer, *lblRemain;
static int nozC, nozT, bedC, bedT, chamC;

static void show_panel(lv_obj_t *o, bool show)
{
    if (show)
    {
        lv_obj_clear_flag(o, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(o, LV_OBJ_FLAG_IGNORE_LAYOUT);
    }
    else
    {
        lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(o, LV_OBJ_FLAG_IGNORE_LAYOUT);
    }
}

static int msg_val(lv_event_t *e, uint32_t *id_out)
{
    lv_msg_t *m = lv_event_get_msg(e);
    if (id_out)
        *id_out = lv_msg_get_id(m);
    const struct XTOUCH_MESSAGE_DATA *d = (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m);
    return d ? (int)d->data : 0;
}

static void temp_cb(lv_event_t *e)
{
    uint32_t id;
    int v = msg_val(e, &id);
    if (id == XTOUCH_ON_NOZZLE_TEMP)
        nozC = v;
    else if (id == XTOUCH_ON_NOZZLE_TARGET_TEMP)
        nozT = v;
    else if (id == XTOUCH_ON_BED_TEMP)
        bedC = v;
    else if (id == XTOUCH_ON_BED_TARGET_TEMP)
        bedT = v;
    else if (id == XTOUCH_ON_CHAMBER_TEMP)
        chamC = v;

    if (lblNozzle)
        lv_label_set_text_fmt(lblNozzle, "%d / %d°", nozC, nozT);
    if (lblBed)
        lv_label_set_text_fmt(lblBed, "%d / %d°", bedC, bedT);
    if (lblChamber)
        lv_label_set_text_fmt(lblChamber, "%d°", chamC);
}

static void light_report_cb(lv_event_t *e)
{
    int on = msg_val(e, NULL);
    if (!swLight)
        return;
    if (on)
        lv_obj_add_state(swLight, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(swLight, LV_STATE_CHECKED);
}

static void light_toggle_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    sendMqttMsg(XTOUCH_COMMAND_LIGHT_TOGGLE, 0);
}

static void pause_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    // Resume if currently paused, otherwise pause.
    if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_PAUSED)
        sendMqttMsg(XTOUCH_COMMAND_RESUME, 0);
    else
        sendMqttMsg(XTOUCH_COMMAND_PAUSE, 0);
}

static void stop_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    sendMqttMsg(XTOUCH_COMMAND_STOP, 0);
}

static void print_status_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    bool printing = (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING ||
                     bambuStatus.print_status == XTOUCH_PRINT_STATUS_PAUSED ||
                     bambuStatus.print_status == XTOUCH_PRINT_STATUS_PREPARE);
    show_panel(printPanel, printing);
    show_panel(readyPanel, !printing);
    if (!printing)
        return;

    int pct = bambuStatus.mc_print_percent;
    if (arcProgress)
        lv_arc_set_value(arcProgress, pct);
    if (lblPct)
        lv_label_set_text_fmt(lblPct, "%d%%", pct);
    if (lblLayer)
        lv_label_set_text_fmt(lblLayer, "LAYER %d/%d", bambuStatus.current_layer, bambuStatus.total_layers);
    if (lblRemain)
        lv_label_set_text_fmt(lblRemain, "%d:%02d", bambuStatus.mc_left_time / 60, bambuStatus.mc_left_time % 60);
}

// small colored dot used as a temp chip icon
static void dot(lv_obj_t *parent, lv_color_t c)
{
    lv_obj_t *d = lv_obj_create(parent);
    lv_obj_remove_style_all(d);
    lv_obj_set_size(d, 12, 12);
    lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(d, c, 0);
    lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
}

// one temp chip; returns the value label so the caller can bind it
static lv_obj_t *temp_chip(lv_obj_t *parent, const char *name, lv_color_t accentDot)
{
    lv_obj_t *chip = lv_obj_create(parent);
    ui5_style_card(chip);
    lv_obj_set_width(chip, LV_PCT(100));
    lv_obj_set_height(chip, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(chip, 13, 0);
    lv_obj_set_style_pad_column(chip, 12, 0);

    dot(chip, accentDot);

    lv_obj_t *col = lv_obj_create(chip);
    lv_obj_remove_style_all(col);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_width(col, LV_SIZE_CONTENT);
    lv_obj_set_height(col, LV_SIZE_CONTENT);

    lv_obj_t *nm = lv_label_create(col);
    lv_label_set_text(nm, name);
    lv_obj_set_style_text_color(nm, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(nm, &lv_font_montserrat_14, 0);

    lv_obj_t *val = lv_label_create(col);
    lv_label_set_text(val, "--");
    lv_obj_set_style_text_color(val, ui_theme_text(), 0);
    lv_obj_set_style_text_font(val, &lv_font_montserrat_24, 0);
    return val;
}

void ui_homeScreen_screen_init(void)
{
    ui_homeScreen = lv_obj_create(NULL);
    lv_obj_t *content = ui5_build_screen(ui_homeScreen, UI5_SCREEN_HOME, "Home");
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(content, 14, 0);

    // ---- left stage ----
    lv_obj_t *stage = lv_obj_create(content);
    ui5_style_card(stage);
    lv_obj_set_flex_grow(stage, 1);
    lv_obj_set_height(stage, LV_PCT(100));
    lv_obj_set_flex_flow(stage, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(stage, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(stage, 16, 0);
    lv_obj_set_style_pad_row(stage, 10, 0);

    // ready panel
    readyPanel = lv_obj_create(stage);
    lv_obj_remove_style_all(readyPanel);
    lv_obj_set_size(readyPanel, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(readyPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(readyPanel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(readyPanel, 10, 0);
    lv_obj_t *ready = lv_label_create(readyPanel);
    lv_label_set_text(ready, LV_SYMBOL_OK "  Ready");
    lv_obj_set_style_text_color(ready, ui_theme_running(), 0);
    lv_obj_set_style_text_font(ready, &lv_font_montserrat_28, 0);
    lv_obj_t *idleSub = lv_label_create(readyPanel);
    lv_label_set_text(idleSub, "Idle");
    lv_obj_set_style_text_color(idleSub, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(idleSub, &lv_font_montserrat_16, 0);

    // printing panel
    printPanel = lv_obj_create(stage);
    lv_obj_remove_style_all(printPanel);
    lv_obj_set_size(printPanel, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(printPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(printPanel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(printPanel, 8, 0);

    arcProgress = lv_arc_create(printPanel);
    lv_obj_set_size(arcProgress, 150, 150);
    lv_arc_set_rotation(arcProgress, 270);
    lv_arc_set_bg_angles(arcProgress, 0, 360);
    lv_arc_set_range(arcProgress, 0, 100);
    lv_arc_set_value(arcProgress, 0);
    lv_obj_remove_style(arcProgress, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arcProgress, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(arcProgress, ui_theme_surface3(), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arcProgress, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arcProgress, ui_theme_running(), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arcProgress, 12, LV_PART_INDICATOR);

    lblPct = lv_label_create(arcProgress);
    lv_label_set_text(lblPct, "0%");
    lv_obj_set_style_text_color(lblPct, ui_theme_text(), 0);
    lv_obj_set_style_text_font(lblPct, &lv_font_montserrat_28, 0);
    lv_obj_center(lblPct);

    lblLayer = lv_label_create(printPanel);
    lv_label_set_text(lblLayer, "LAYER 0/0");
    lv_obj_set_style_text_color(lblLayer, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(lblLayer, &lv_font_montserrat_14, 0);

    lblRemain = lv_label_create(printPanel);
    lv_label_set_text(lblRemain, "0:00");
    lv_obj_set_style_text_color(lblRemain, ui_theme_text(), 0);
    lv_obj_set_style_text_font(lblRemain, &lv_font_montserrat_20, 0);

    lv_obj_t *btnRow = lv_obj_create(printPanel);
    lv_obj_remove_style_all(btnRow);
    lv_obj_set_size(btnRow, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btnRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btnRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btnRow, 10, 0);
    lv_obj_set_style_pad_top(btnRow, 6, 0);

    lv_obj_t *bPause = lv_btn_create(btnRow);
    lv_obj_set_style_bg_color(bPause, ui_theme_surface2(), 0);
    lv_obj_set_style_border_color(bPause, ui_theme_pause(), 0);
    lv_obj_set_style_border_width(bPause, 1, 0);
    lv_obj_add_event_cb(bPause, pause_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bPauseL = lv_label_create(bPause);
    lv_label_set_text(bPauseL, LV_SYMBOL_PAUSE " Pause");
    lv_obj_set_style_text_color(bPauseL, ui_theme_pause(), 0);
    lv_obj_center(bPauseL);

    lv_obj_t *bStop = lv_btn_create(btnRow);
    lv_obj_set_style_bg_color(bStop, ui_theme_surface2(), 0);
    lv_obj_set_style_border_color(bStop, ui_theme_stop(), 0);
    lv_obj_set_style_border_width(bStop, 1, 0);
    lv_obj_add_event_cb(bStop, stop_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bStopL = lv_label_create(bStop);
    lv_label_set_text(bStopL, LV_SYMBOL_STOP " Stop");
    lv_obj_set_style_text_color(bStopL, ui_theme_stop(), 0);
    lv_obj_center(bStopL);

    // subscribe the stage to print status (arc/labels updated from bambuStatus)
    lv_obj_add_event_cb(stage, print_status_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_PRINT_STATUS, stage, NULL);

    // ---- right column ----
    lv_obj_t *side = lv_obj_create(content);
    lv_obj_remove_style_all(side);
    lv_obj_set_width(side, 240);
    lv_obj_set_height(side, LV_PCT(100));
    lv_obj_set_flex_flow(side, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(side, 10, 0);

    lblNozzle = temp_chip(side, "Nozzle", ui_theme_heat());
    lv_obj_add_event_cb(lblNozzle, temp_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_NOZZLE_TEMP, lblNozzle, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_NOZZLE_TARGET_TEMP, lblNozzle, NULL);

    lblBed = temp_chip(side, "Bed", ui_theme_heat());
    lv_obj_add_event_cb(lblBed, temp_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_BED_TEMP, lblBed, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_BED_TARGET_TEMP, lblBed, NULL);

    lblChamber = temp_chip(side, "Chamber", ui_theme_chamber());
    lv_obj_add_event_cb(lblChamber, temp_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_CHAMBER_TEMP, lblChamber, NULL);

    // light toggle row
    lv_obj_t *lightRow = lv_obj_create(side);
    ui5_style_card(lightRow);
    lv_obj_set_width(lightRow, LV_PCT(100));
    lv_obj_set_height(lightRow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(lightRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(lightRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(lightRow, 13, 0);
    lv_obj_t *lightName = lv_label_create(lightRow);
    lv_label_set_text(lightName, "Chamber light");
    lv_obj_set_style_text_color(lightName, ui_theme_text(), 0);
    lv_obj_set_style_text_font(lightName, &lv_font_montserrat_14, 0);

    swLight = lv_switch_create(lightRow);
    lv_obj_set_style_bg_color(swLight, ui_theme_accent(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(swLight, light_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(swLight, light_report_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_LIGHT_REPORT, swLight, NULL);

    // default: assume idle until a print-status message arrives
    show_panel(printPanel, false);
    show_panel(readyPanel, true);
}
