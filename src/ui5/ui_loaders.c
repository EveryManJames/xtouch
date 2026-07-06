#include "ui.h"

void sendMqttMsg(int message, uint32_t data)
{
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = data;
    lv_msg_send(message, &eventData);
}

// Re-broadcast the relevant slice of bambuStatus so a freshly-built screen
// populates immediately instead of waiting for the next MQTT push.
void fillScreenData(int screen)
{
    switch (screen)
    {
    case UI5_SCREEN_HOME:
        sendMqttMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
        sendMqttMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
        sendMqttMsg(XTOUCH_ON_LIGHT_REPORT, bambuStatus.chamberLed);
        sendMqttMsg(XTOUCH_ON_AMS, bambuStatus.ams);
        sendMqttMsg(XTOUCH_ON_PRINT_STATUS, 0);
        sendMqttMsg(XTOUCH_ON_CHAMBER_TEMP, bambuStatus.chamber_temper);
        break;
    case UI5_SCREEN_TEMP:
        sendMqttMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
        sendMqttMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
        sendMqttMsg(XTOUCH_ON_PART_FAN_SPEED, bambuStatus.cooling_fan_speed);
        sendMqttMsg(XTOUCH_ON_PART_AUX_SPEED, bambuStatus.big_fan1_speed);
        sendMqttMsg(XTOUCH_ON_PART_CHAMBER_SPEED, bambuStatus.big_fan2_speed);
        break;
    case UI5_SCREEN_CONTROL:
        sendMqttMsg(XTOUCH_CONTROL_INC_SWITCH, controlMode.inc);
        break;
    case UI5_SCREEN_FILAMENT:
        sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
        break;
    }
}

void loadScreen(int screen)
{
    xTouchConfig.currentScreenIndex = screen;
    lv_obj_t *current = lv_scr_act();
    if (current != NULL)
    {
        lv_obj_clean(current);
        lv_obj_del(current);
    }

    switch (screen)
    {
    case -1:
        ui_introScreen_screen_init();
        lv_disp_load_scr(introScreen);
        break;
    case UI5_SCREEN_HOME:
        ui_homeScreen_screen_init();
        lv_disp_load_scr(ui_homeScreen);
        break;
    case UI5_SCREEN_TEMP:
        ui_temperatureScreen_screen_init();
        lv_disp_load_scr(ui_temperatureScreen);
        break;
    case UI5_SCREEN_CONTROL:
        ui_controlScreen_screen_init();
        lv_disp_load_scr(ui_controlScreen);
        break;
    case UI5_SCREEN_FILAMENT:
        ui_filamentScreen_screen_init();
        lv_disp_load_scr(ui_filamentScreen);
        break;
    case UI5_SCREEN_SETTINGS:
        ui_settingsScreen_screen_init();
        lv_disp_load_scr(ui_settingsScreen);
        break;
    case UI5_SCREEN_ACCESSORIES:
        ui_accessoriesScreen_screen_init();
        lv_disp_load_scr(ui_accessoriesScreen);
        break;
    }

    fillScreenData(screen);
}
