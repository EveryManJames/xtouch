#ifndef _XTOUCH_UI5_UI_H
#define _XTOUCH_UI5_UI_H

// Hand-authored 800x480 landscape UI for the 5" Guition JC8048W550 build,
// selected when __XTOUCH_SCREEN_5__ is defined. It replaces the SquareLine
// 320x240 tree in src/ui/ but keeps the exact same public contract the rest of
// the firmware depends on (intro globals, loadScreen/initTopLayer, the three
// settings widgets read by events.h, LV_EVENT_GET_COMP_CHILD) plus the shared
// message-bus enum (ui/ui_msgs.h) and error helpers (xtouch/errors.h).

#include <Arduino.h>
#include "xtouch/errors.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "xtouch/types.h"

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui/ui_msgs.h"
#include "ui_theme.h"

  // Screen indices used by loadScreen() and the nav rail.
  enum UI5_SCREEN
  {
    UI5_SCREEN_HOME = 0,
    UI5_SCREEN_TEMP = 1,
    UI5_SCREEN_CONTROL = 2,
    UI5_SCREEN_FILAMENT = 3,
    UI5_SCREEN_SETTINGS = 4,
    UI5_SCREEN_ACCESSORIES = 5,
    UI5_SCREEN_COUNT
  };

  // Timers owned by the device screen driver (assigned in devices/5.0/screen.h).
  lv_timer_t *xtouch_screen_onScreenOffTimer;
  lv_timer_t *xtouch_ssdp_onButtonTimerTimer;

  // Assigned by the device driver via lv_event_register_id().
  extern uint32_t LV_EVENT_GET_COMP_CHILD;

  // --- boot / intro screen (referenced by coldboot/connection/firmware/mqtt/sdcard) ---
  lv_obj_t *introScreen;
  lv_obj_t *introScreenIcon;
  lv_obj_t *introScreenCaption;

  // --- per-screen roots ---
  lv_obj_t *ui_homeScreen;
  lv_obj_t *ui_temperatureScreen;
  lv_obj_t *ui_controlScreen;
  lv_obj_t *ui_filamentScreen;
  lv_obj_t *ui_settingsScreen;
  lv_obj_t *ui_accessoriesScreen;

  // --- settings widgets read directly by xtouch/events.h ---
  lv_obj_t *ui_settingsBackLightPanelSlider;
  lv_obj_t *ui_settingsTFTOFFSlider;
  lv_obj_t *ui_settingsTFTInvertSwitch;

  // top-layer overlays (created in initTopLayer)
  lv_obj_t *ui_confirmComponent;
  lv_obj_t *ui_hmsComponent;

  // screen builders
  void ui_introScreen_screen_init(void);
  void ui_homeScreen_screen_init(void);
  void ui_temperatureScreen_screen_init(void);
  void ui_controlScreen_screen_init(void);
  void ui_filamentScreen_screen_init(void);
  void ui_settingsScreen_screen_init(void);
  void ui_accessoriesScreen_screen_init(void);

  void loadScreen(int screen);
  void initTopLayer(void);
  void fillScreenData(int screen);
  void sendMqttMsg(int message, uint32_t data);
  void ui_init(void);

  // --- shared chrome (src/ui5/ui_chrome.c) ---
  // Builds nav rail + status strip on `screenRoot`, marks nav[index] active, and
  // returns the padded content container the caller should populate.
  lv_obj_t *ui5_build_screen(lv_obj_t *screenRoot, int index, const char *title);
  void ui5_style_card(lv_obj_t *obj);       // surface + hairline + radius
  void ui5_apply_screen_bg(lv_obj_t *root); // page background for a screen root

  // defined in xtouch/bblp.h
  bool xtouch_bblp_is_p1p();
  bool xtouch_bblp_is_p1Series();
  bool xtouch_bblp_is_x1();
  bool xtouch_bblp_is_x1s();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
