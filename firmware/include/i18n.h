#ifndef ATHENAOS_I18N_H
#define ATHENAOS_I18N_H

#include <Arduino.h>

enum I18nId : uint8_t {
    I18N_SETTINGS = 0,
    I18N_VOLUME,
    I18N_BRIGHTNESS,
    I18N_BRI_LOW,
    I18N_BRI_MID,
    I18N_BRI_HIGH,
    I18N_WIFI,
    I18N_OFF,
    I18N_CONNECTED,
    I18N_NOT_CONNECTED,
    I18N_RADIO_OFF,
    I18N_NETWORK,
    I18N_SETUP_HOTSPOT,
    I18N_JOIN_WIFI,
    I18N_OPEN_PHONE,
    I18N_PICK_WIFI,
    I18N_HOTSPOT_ON,
    I18N_START_HOTSPOT,
    I18N_STOP_HOTSPOT,
    I18N_HOTSPOT_HINT,
    I18N_RESTART,
    I18N_POWER_OFF,
    I18N_RESTART_Q,
    I18N_RESTART_HINT,
    I18N_POWER_Q,
    I18N_POWER_HINT,
    I18N_WIFI_OK,
    I18N_WIFI_OK_HINT,
    I18N_YES,
    I18N_NO,
    I18N_NTP_OK,
    I18N_NTP_WAIT,
    I18N_NO_APPS,
    I18N_TAP_OPEN,
    I18N_HOLD_HOME,
    I18N_FACTORY,
    I18N_FACTORY_Q,
    I18N_FACTORY_HINT,
    I18N_LANGUAGE,
    I18N_SOUND,
    I18N_MIC,
    I18N_PLAY,
    I18N_PLAYING,
    I18N_TEST,
    I18N_LISTENING,
    I18N_NA,
    I18N_COUNT
};

class I18n {
public:
    static const char *t(I18nId id);
    static int langCount();
    static const char *langId(int index);
    static const char *langLabel(int index);
    static int langIndex(const char *id);
};

#endif
