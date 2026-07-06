#include "ui.h"

// Assigned by the device driver (devices/5.0/screen.h) via lv_event_register_id().
uint32_t LV_EVENT_GET_COMP_CHILD;

void initTopLayer(void)
{
    // Hidden overlay containers on the top layer, reused later for the
    // confirm dialog and HMS error banner. Kept minimal for now.
    ui_confirmComponent = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(ui_confirmComponent);
    lv_obj_set_size(ui_confirmComponent, 800, 480);
    lv_obj_add_flag(ui_confirmComponent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_confirmComponent, LV_OBJ_FLAG_CLICKABLE);

    ui_hmsComponent = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(ui_hmsComponent);
    lv_obj_set_size(ui_hmsComponent, 800, 480);
    lv_obj_add_flag(ui_hmsComponent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_hmsComponent, LV_OBJ_FLAG_CLICKABLE);
}

void ui_init(void)
{
    ui_theme_init();
    loadScreen(UI5_SCREEN_HOME);
}
