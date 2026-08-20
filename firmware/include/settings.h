#ifndef ATHENAOS_SETTINGS_H
#define ATHENAOS_SETTINGS_H

#include <Arduino.h>

class Settings {
public:
    static void begin();
    static void poll();
    static void applyAudio();
    static void applyWifi();

    static uint8_t volume();
    static void setVolume(uint8_t percent);

    static bool wifiEnabled();
    static void setWifiEnabled(bool on);
    static bool hasWifiConfig();
    static const char *wifiSsid();
    static const char *wifiPassword();
    static void setWifi(const char *ssid, const char *password);

    static bool wifiConnected();
    static bool apActive();
    static const char *apSsid();
    static void startAp();
    static void stopAp();
    static void setApEnabled(bool on);
    static bool takeRestartAsk();

    static const char *tzId();
    static const char *tzPosix();
    static void setTzId(const char *id);
    static void nextTz();
    static bool ntpEnabled();
    static void setNtpEnabled(bool on);
    static bool ntpSynced();

    static const char *lang();
    static const char *langLabel();
    static void setLang(const char *id);
    static void nextLang();
    static void factoryReset();

private:
    static void load();
    static void save();

    static bool _loaded;
    static uint8_t _volume;
    static bool _wifiOn;
    static char _ssid[33];
    static char _pass[65];
    static char _tz[16];
    static bool _ntpOn;
    static char _lang[4];
};

#endif
