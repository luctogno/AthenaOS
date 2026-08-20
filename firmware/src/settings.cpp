#include "settings.h"
#include "audio.h"
#include "boards/board.h"
#include "clock.h"

#include <Preferences.h>
#include <WiFi.h>
#include <time.h>
#include <string.h>

bool Settings::_loaded = false;
uint8_t Settings::_volume = 70;
bool Settings::_wifiOn = true;
char Settings::_ssid[33] = "";
char Settings::_pass[65] = "";
char Settings::_tz[16] = "Rome";
bool Settings::_ntpOn = true;
char Settings::_lang[4] = "en";

static bool apOn = false;
static bool wasConnected = false;
static bool restartAsk = false;
static bool ntpStarted = false;

struct TzPreset {
    const char *id;
    const char *posix;
};

static const TzPreset TZ_PRESETS[] = {
    {"Rome", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"UTC", "UTC0"},
    {"London", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"NewYork", "EST5EDT,M3.2.0,M11.1.0"},
    {"Tokyo", "JST-9"},
};
static const int TZ_COUNT = 5;

static const TzPreset *tzById(const char *id) {
    for (int i = 0; i < TZ_COUNT; i++) {
        if (strcmp(TZ_PRESETS[i].id, id) == 0) return &TZ_PRESETS[i];
    }
    return &TZ_PRESETS[0];
}

static Preferences prefs;

void Settings::load() {
    if (!prefs.begin("athena", true)) {
        DEBUG_PRINTLN("[Settings] NVS open-ro failed, using defaults");
        return;
    }
    _volume = prefs.getUChar("vol", 70);
    if (_volume > 100) _volume = 100;
    _wifiOn = prefs.getBool("wifion", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    String tz = prefs.getString("tz", "Rome");
    _ntpOn = prefs.getBool("ntp", true);
    String lang = prefs.getString("lang", "en");
    prefs.end();
    strncpy(_ssid, ssid.c_str(), sizeof(_ssid) - 1);
    _ssid[sizeof(_ssid) - 1] = 0;
    strncpy(_pass, pass.c_str(), sizeof(_pass) - 1);
    _pass[sizeof(_pass) - 1] = 0;
    strncpy(_tz, tzById(tz.c_str())->id, sizeof(_tz) - 1);
    _tz[sizeof(_tz) - 1] = 0;
    setLang(lang.c_str());
}

void Settings::save() {
    if (!prefs.begin("athena", false)) {
        DEBUG_PRINTLN("[Settings] NVS open-rw failed");
        return;
    }
    prefs.putUChar("vol", _volume);
    prefs.putBool("wifion", _wifiOn);
    prefs.putString("ssid", _ssid);
    prefs.putString("pass", _pass);
    prefs.putString("tz", _tz);
    prefs.putBool("ntp", _ntpOn);
    prefs.putString("lang", _lang);
    prefs.end();
}

void Settings::startAp() {
    if (apOn || !_wifiOn) return;
    WiFi.persistent(false);
    WiFi.softAP(WIFI_AP_SSID);
    apOn = true;
    DEBUG_PRINTF("[Settings] AP '%s' %s\n",
                 WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());
}

void Settings::stopAp() {
    WiFi.softAPdisconnect(true);
    WiFi.enableAP(false);
    apOn = false;
    if (_wifiOn) WiFi.mode(WIFI_STA);
    DEBUG_PRINTLN("[Settings] AP off");
}

void Settings::setApEnabled(bool on) {
    if (on) startAp();
    else stopAp();
}

bool Settings::apActive() {
    return apOn;
}

const char *Settings::apSsid() {
    return WIFI_AP_SSID;
}

bool Settings::wifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool Settings::takeRestartAsk() {
    if (!restartAsk) return false;
    restartAsk = false;
    return true;
}

void Settings::begin() {
    if (_loaded) return;
    load();
    _loaded = true;
    DEBUG_PRINTF("[Settings] vol=%u wifi=%d ssid='%s' tz=%s ntp=%d lang=%s\n",
                 (unsigned)_volume, _wifiOn, _ssid, _tz, _ntpOn, _lang);
    applyAudio();
    WiFi.persistent(false);
    apOn = false;
    WiFi.mode(WIFI_STA);
    WiFi.enableAP(false);
    WiFi.softAPdisconnect(true);
    applyWifi();
}

void Settings::poll() {
    if (!_wifiOn) return;

    bool connected = wifiConnected();
    if (connected && !wasConnected && apOn) restartAsk = true;
    wasConnected = connected;
    if (connected && _ntpOn && !ntpStarted) {
        const char *posix = tzById(_tz)->posix;
        configTzTime(posix, "it.pool.ntp.org", "europe.pool.ntp.org", "pool.ntp.org");
        ntpStarted = true;
        DEBUG_PRINTF("[Settings] NTP %s (%s)\n", _tz, posix);
    }
}

void Settings::applyAudio() {
    Audio::setVolume(_volume);
}

void Settings::applyWifi() {
    WiFi.persistent(false);
    if (!_wifiOn) {
        stopAp();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        DEBUG_PRINTLN("[Settings] WiFi off");
        return;
    }
    if (apOn) {
        WiFi.softAP(WIFI_AP_SSID);
    } else {
        WiFi.mode(WIFI_STA);
        WiFi.enableAP(false);
        WiFi.softAPdisconnect(true);
    }
    if (!_ssid[0]) {
        DEBUG_PRINTLN("[Settings] WiFi on, no STA config");
        return;
    }
    DEBUG_PRINTF("[Settings] WiFi begin SSID='%s'\n", _ssid);
    WiFi.begin(_ssid, _pass);
}

uint8_t Settings::volume() {
    return _volume;
}

void Settings::setVolume(uint8_t percent) {
    if (percent > 100) percent = 100;
    _volume = percent;
    save();
    applyAudio();
}

bool Settings::wifiEnabled() {
    return _wifiOn;
}

void Settings::setWifiEnabled(bool on) {
    _wifiOn = on;
    save();
    applyWifi();
}

bool Settings::hasWifiConfig() {
    return _ssid[0] != 0;
}

const char *Settings::wifiSsid() {
    return _ssid;
}

const char *Settings::wifiPassword() {
    return _pass;
}

void Settings::setWifi(const char *ssid, const char *password) {
    strncpy(_ssid, ssid ? ssid : "", sizeof(_ssid) - 1);
    _ssid[sizeof(_ssid) - 1] = 0;
    strncpy(_pass, password ? password : "", sizeof(_pass) - 1);
    _pass[sizeof(_pass) - 1] = 0;
    save();
    if (_wifiOn) applyWifi();
}

const char *Settings::tzId() {
    return _tz;
}

const char *Settings::tzPosix() {
    return tzById(_tz)->posix;
}

void Settings::setTzId(const char *id) {
    strncpy(_tz, tzById(id ? id : "Rome")->id, sizeof(_tz) - 1);
    _tz[sizeof(_tz) - 1] = 0;
    save();
    ntpStarted = false;
    Clock::applyTz();
}

void Settings::nextTz() {
    int i = 0;
    for (; i < TZ_COUNT; i++) {
        if (strcmp(TZ_PRESETS[i].id, _tz) == 0) break;
    }
    setTzId(TZ_PRESETS[(i + 1) % TZ_COUNT].id);
}

bool Settings::ntpEnabled() {
    return _ntpOn;
}

void Settings::setNtpEnabled(bool on) {
    _ntpOn = on;
    save();
    ntpStarted = false;
}

bool Settings::ntpSynced() {
    return Clock::hasTime();
}

static bool langIsItValue(const char *id) {
    return id && (id[0] == 'i' || id[0] == 'I') && (id[1] == 't' || id[1] == 'T');
}

const char *Settings::lang() {
    return langIsIt() ? "it" : "en";
}

bool Settings::langIsIt() {
    return langIsItValue(_lang);
}

void Settings::setLang(const char *id) {
    const char *v = langIsItValue(id) ? "it" : "en";
    if (strcmp(_lang, v) == 0) return;
    strncpy(_lang, v, sizeof(_lang) - 1);
    _lang[sizeof(_lang) - 1] = 0;
    if (_loaded) save();
}

void Settings::toggleLang() {
    setLang(langIsIt() ? "en" : "it");
}
