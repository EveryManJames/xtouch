#include "ui.h"

// Four tiles: nozzle, bed (°C) and part/aux fan (%). Each shows a live current
// value and an adjustable target; +/- sends the matching command over the bus.
static lv_obj_t *lblNozzleCur, *lblBedCur, *lblPartCur, *lblAuxCur;
static lv_obj_t *lblNozzleTgt, *lblBedTgt, *lblPartTgt, *lblAuxTgt;
static int tgtNozzle, tgtBed, tgtPart, tgtAux;

static int msg_int(lv_event_t *e)
{
    const struct XTOUCH_MESSAGE_DATA *d = (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(lv_event_get_msg(e));
    return d ? (int)d->data : 0;
}
static void nozzleCur_cb(lv_event_t *e) { if (lblNozzleCur) lv_label_set_text_fmt(lblNozzleCur, "%d°", msg_int(e)); }
static void bedCur_cb(lv_event_t *e) { if (lblBedCur) lv_label_set_text_fmt(lblBedCur, "%d°", msg_int(e)); }
static void partCur_cb(lv_event_t *e) { if (lblPartCur) lv_label_set_text_fmt(lblPartCur, "%d%%", msg_int(e)); }
static void auxCur_cb(lv_event_t *e) { if (lblAuxCur) lv_label_set_text_fmt(lblAuxCur, "%d%%", msg_int(e)); }

static void clampi(int *v, int lo, int hi) { if (*v < lo) *v = lo; if (*v > hi) *v = hi; }

static void nozzle_step(lv_event_t *e) { tgtNozzle += (int)(intptr_t)lv_event_get_user_data(e); clampi(&tgtNozzle, 0, 300); lv_label_set_text_fmt(lblNozzleTgt, "%d°", tgtNozzle); sendMqttMsg(XTOUCH_COMMAND_NOZZLE_TARGET_TEMP, tgtNozzle); }
static void bed_step(lv_event_t *e) { tgtBed += (int)(intptr_t)lv_event_get_user_data(e); clampi(&tgtBed, 0, 120); lv_label_set_text_fmt(lblBedTgt, "%d°", tgtBed); sendMqttMsg(XTOUCH_COMMAND_BED_TARGET_TEMP, tgtBed); }
static void part_step(lv_event_t *e) { tgtPart += (int)(intptr_t)lv_event_get_user_data(e); clampi(&tgtPart, 0, 100); lv_label_set_text_fmt(lblPartTgt, "%d%%", tgtPart); sendMqttMsg(XTOUCH_COMMAND_PART_FAN_SPEED, tgtPart); }
static void aux_step(lv_event_t *e) { tgtAux += (int)(intptr_t)lv_event_get_user_data(e); clampi(&tgtAux, 0, 100); lv_label_set_text_fmt(lblAuxTgt, "%d%%", tgtAux); sendMqttMsg(XTOUCH_COMMAND_AUX_FAN_SPEED, tgtAux); }

// Builds a tile; fills *cur and *tgt with the current/target labels.
static void tile(lv_obj_t *grid, const char *name, lv_color_t dotc, int step,
                 lv_event_cb_t stepCb, lv_obj_t **cur, lv_obj_t **tgt, const char *tgtInit)
{
    lv_obj_t *t = lv_obj_create(grid);
    ui5_style_card(t);
    lv_obj_set_flex_flow(t, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(t, 14, 0);
    lv_obj_set_style_pad_row(t, 6, 0);

    lv_obj_t *nm = lv_label_create(t);
    lv_label_set_text(nm, name);
    lv_obj_set_style_text_color(nm, dotc, 0);
    lv_obj_set_style_text_font(nm, &lv_font_montserrat_14, 0);

    *cur = lv_label_create(t);
    lv_label_set_text(*cur, "--");
    lv_obj_set_style_text_color(*cur, ui_theme_text(), 0);
    lv_obj_set_style_text_font(*cur, &lv_font_montserrat_36, 0);

    lv_obj_t *ctl = lv_obj_create(t);
    lv_obj_remove_style_all(ctl);
    lv_obj_set_width(ctl, LV_PCT(100));
    lv_obj_set_height(ctl, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ctl, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctl, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *minus = lv_btn_create(ctl);
    lv_obj_set_style_bg_color(minus, ui_theme_surface2(), 0);
    lv_obj_add_event_cb(minus, stepCb, LV_EVENT_CLICKED, (void *)(intptr_t)(-step));
    lv_obj_t *ml = lv_label_create(minus);
    lv_label_set_text(ml, LV_SYMBOL_MINUS);
    lv_obj_set_style_text_color(ml, ui_theme_text(), 0);
    lv_obj_center(ml);

    *tgt = lv_label_create(ctl);
    lv_label_set_text(*tgt, tgtInit);
    lv_obj_set_style_text_color(*tgt, ui_theme_muted(), 0);
    lv_obj_set_style_text_font(*tgt, &lv_font_montserrat_20, 0);

    lv_obj_t *plus = lv_btn_create(ctl);
    lv_obj_set_style_bg_color(plus, ui_theme_accent(), 0);
    lv_obj_add_event_cb(plus, stepCb, LV_EVENT_CLICKED, (void *)(intptr_t)(step));
    lv_obj_t *pl = lv_label_create(plus);
    lv_label_set_text(pl, LV_SYMBOL_PLUS);
    lv_obj_set_style_text_color(pl, ui_theme_on_accent(), 0);
    lv_obj_center(pl);
}

void ui_temperatureScreen_screen_init(void)
{
    ui_temperatureScreen = lv_obj_create(NULL);
    lv_obj_t *content = ui5_build_screen(ui_temperatureScreen, UI5_SCREEN_TEMP, "Temperature & Fans");

    tgtNozzle = bambuStatus.nozzle_target_temper;
    tgtBed = bambuStatus.bed_target_temper;
    tgtPart = bambuStatus.cooling_fan_speed;
    tgtAux = bambuStatus.big_fan1_speed;

    // 2x2 grid
    static lv_coord_t cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t rows[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(content, cols, rows);
    lv_obj_set_layout(content, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(content, 12, 0);
    lv_obj_set_style_pad_column(content, 12, 0);

    char buf[8];
    tile(content, "Nozzle", ui_theme_heat(), 5, nozzle_step, &lblNozzleCur, &lblNozzleTgt, (snprintf(buf, sizeof buf, "%d°", tgtNozzle), buf));
    lv_obj_set_grid_cell(lv_obj_get_child(content, 0), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    tile(content, "Bed", ui_theme_heat(), 5, bed_step, &lblBedCur, &lblBedTgt, (snprintf(buf, sizeof buf, "%d°", tgtBed), buf));
    lv_obj_set_grid_cell(lv_obj_get_child(content, 1), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    tile(content, "Part fan", ui_theme_chamber(), 10, part_step, &lblPartCur, &lblPartTgt, (snprintf(buf, sizeof buf, "%d%%", tgtPart), buf));
    lv_obj_set_grid_cell(lv_obj_get_child(content, 2), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    tile(content, "Aux fan", ui_theme_chamber(), 10, aux_step, &lblAuxCur, &lblAuxTgt, (snprintf(buf, sizeof buf, "%d%%", tgtAux), buf));
    lv_obj_set_grid_cell(lv_obj_get_child(content, 3), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_add_event_cb(lblNozzleCur, nozzleCur_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_NOZZLE_TEMP, lblNozzleCur, NULL);
    lv_obj_add_event_cb(lblBedCur, bedCur_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_BED_TEMP, lblBedCur, NULL);
    lv_obj_add_event_cb(lblPartCur, partCur_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_PART_FAN_SPEED, lblPartCur, NULL);
    lv_obj_add_event_cb(lblAuxCur, auxCur_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subscribe_obj(XTOUCH_ON_PART_AUX_SPEED, lblAuxCur, NULL);
}
