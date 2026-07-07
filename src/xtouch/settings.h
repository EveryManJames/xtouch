#ifndef _XLCD_SETTINGS
#define _XLCD_SETTINGS

void xtouch_settings_save(bool onlyRoot = false)
{
    DynamicJsonDocument doc(512);
    doc["backlight"] = xTouchConfig.xTouchBacklightLevel;
    doc["tftOff"] = xTouchConfig.xTouchTFTOFFValue;
    doc["tftInvert"] = xTouchConfig.xTouchTFTInvert;
    doc["ota"] = xTouchConfig.xTouchOTAEnabled;
    doc["wop"] = xTouchConfig.xTouchWakeOnPrint;
    doc["chamberTempDiff"] = xTouchConfig.xTouchChamberSensorReadingDiff;
    doc["chamberTemp"] = xTouchConfig.xTouchChamberSensorEnabled;
    doc["auxFan"] = xTouchConfig.xTouchAuxFanEnabled;
    doc["chamberFan"] = xTouchConfig.xTouchChamberFanEnabled;
    doc["themeAccent"] = xTouchConfig.xTouchThemeAccent;
    doc["themeDark"] = xTouchConfig.xTouchThemeDark;
    doc["ledEnabled"] = xTouchConfig.xTouchLedEnabled;
    doc["ledGpio"] = xTouchConfig.xTouchLedGpio;
    doc["ledCount"] = xTouchConfig.xTouchLedCount;
    doc["ledBrightness"] = xTouchConfig.xTouchLedBrightness;
    doc["ledMode"] = xTouchConfig.xTouchLedMode;
    doc["ledColor"] = xTouchConfig.xTouchLedColor;

    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, doc);
}

void xtouch_settings_loadSettings()
{
    if (!xtouch_filesystem_exist(SD, xtouch_paths_settings))
    {
        DynamicJsonDocument doc(256);
        xTouchConfig.xTouchBacklightLevel = 128;
        xTouchConfig.xTouchTFTOFFValue = 15;
        xTouchConfig.xTouchTFTInvert = false;
        xTouchConfig.xTouchOTAEnabled = false;
        xTouchConfig.xTouchWakeOnPrint = true;
        xTouchConfig.xTouchChamberSensorReadingDiff = 0;
        xTouchConfig.xTouchChamberSensorEnabled = false;
        xTouchConfig.xTouchAuxFanEnabled = false;
        xTouchConfig.xTouchChamberFanEnabled = false;
        xTouchConfig.xTouchThemeAccent = 0;
        xTouchConfig.xTouchThemeDark = true;
        xTouchConfig.xTouchLedEnabled = false;
        xTouchConfig.xTouchLedGpio = 18;
        xTouchConfig.xTouchLedCount = 12;
        xTouchConfig.xTouchLedBrightness = 128;
        xTouchConfig.xTouchLedMode = 4; // status-sync
        xTouchConfig.xTouchLedColor = 0x35D0BA;
        xtouch_settings_save(true);
    }

    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);

    xTouchConfig.xTouchBacklightLevel = settings.containsKey("backlight") ? settings["backlight"].as<int>() : 128;
    xTouchConfig.xTouchTFTOFFValue = settings.containsKey("tftOff") ? settings["tftOff"].as<int>() : 15;
    xTouchConfig.xTouchTFTInvert = settings.containsKey("tftInvert") ? settings["tftInvert"].as<bool>() : false;
    xTouchConfig.xTouchOTAEnabled = settings.containsKey("ota") ? settings["ota"].as<bool>() : false;
    xTouchConfig.xTouchWakeOnPrint = settings.containsKey("wop") ? settings["wop"].as<bool>() : true;
    xTouchConfig.xTouchChamberSensorReadingDiff = settings.containsKey("chamberTempDiff") ? settings["chamberTempDiff"].as<int8_t>() : 0;
    xTouchConfig.xTouchChamberSensorEnabled = settings.containsKey("chamberTemp") ? settings["chamberTemp"].as<bool>() : false;
    xTouchConfig.xTouchAuxFanEnabled = settings.containsKey("auxFan") ? settings["auxFan"].as<bool>() : false;
    xTouchConfig.xTouchChamberFanEnabled = settings.containsKey("chamberFan") ? settings["chamberFan"].as<bool>() : false;
    xTouchConfig.xTouchThemeAccent = settings.containsKey("themeAccent") ? settings["themeAccent"].as<uint8_t>() : 0;
    xTouchConfig.xTouchThemeDark = settings.containsKey("themeDark") ? settings["themeDark"].as<bool>() : true;
    xTouchConfig.xTouchLedEnabled = settings.containsKey("ledEnabled") ? settings["ledEnabled"].as<bool>() : false;
    xTouchConfig.xTouchLedGpio = settings.containsKey("ledGpio") ? settings["ledGpio"].as<uint8_t>() : 18;
    xTouchConfig.xTouchLedCount = settings.containsKey("ledCount") ? settings["ledCount"].as<uint16_t>() : 12;
    xTouchConfig.xTouchLedBrightness = settings.containsKey("ledBrightness") ? settings["ledBrightness"].as<uint8_t>() : 128;
    xTouchConfig.xTouchLedMode = settings.containsKey("ledMode") ? settings["ledMode"].as<uint8_t>() : 4;
    xTouchConfig.xTouchLedColor = settings.containsKey("ledColor") ? settings["ledColor"].as<uint32_t>() : 0x35D0BA;

    xtouch_screen_setupTFTFlip();
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);

    xtouch_screen_invertColors();
}

#endif