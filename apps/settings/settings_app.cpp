#include "app_manager.h"
#include "display.h"
#include "boards/board.h"
#include "icon.h"
#include "settings.h"
#include "status_bar.h"
#include "power.h"
#include "i18n.h"
#include "audio.h"
#include "mic.h"

#include <WiFi.h>
#include <stdio.h>
#include <string.h>

class SettingsApp : public App {
public:
    AppManifest getManifest() override {
        return {"settings", I18n::t(I18N_SETTINGS), "1.0.0", "AthenaOS", true, drawSettingsIcon, nullptr, 0, 0};
    }

    void start() override {
        state = STATE_RUNNING;
        _confirm = CONFIRM_NONE;
        _wifiWasOk = Settings::wifiConnected();
        _apWasOn = Settings::apActive();
        strncpy(_langShown, Settings::lang(), sizeof(_langShown) - 1);
        _langShown[sizeof(_langShown) - 1] = 0;
        _sndOn = Audio::testActive();
        _micOn = Mic::testActive();
        _dirty = true;
    }

    void resume() override {
        start();
    }

    void update() override {
        if (state != STATE_RUNNING) return;
        if (Settings::takeRestartAsk()) {
            _confirm = CONFIRM_WIFI;
            _dirty = true;
        }
        bool ok = Settings::wifiConnected();
        if (ok != _wifiWasOk) {
            _wifiWasOk = ok;
            _dirty = true;
        }
        bool ap = Settings::apActive();
        if (ap != _apWasOn) {
            _apWasOn = ap;
            _dirty = true;
        }
        if (strcmp(_langShown, Settings::lang()) != 0) {
            strncpy(_langShown, Settings::lang(), sizeof(_langShown) - 1);
            _langShown[sizeof(_langShown) - 1] = 0;
            _dirty = true;
        }
        bool snd = Audio::testActive();
        bool mic = Mic::testActive();
        if (snd != _sndOn || mic != _micOn) {
            _sndOn = snd;
            _micOn = mic;
            _dirty = true;
        } else if (mic) {
            unsigned long now = millis();
            if (now - _micDrawMs >= 100) {
                _micDrawMs = now;
                _dirty = true;
            }
        }
    }

    void draw() override {
        if (!_dirty) return;
        _dirty = false;
        render();
    }

    void onTouchDown(uint16_t x, uint16_t y) override {
        if (_confirm != CONFIRM_NONE) {
            if (hit(x, y, 24, 280, 150, 52)) {
                if (_confirm == CONFIRM_POWER) Power::off();
                else if (_confirm == CONFIRM_FACTORY) Settings::factoryReset();
                else Power::restart();
            } else if (hit(x, y, 194, 280, 150, 52)) {
                _confirm = CONFIRM_NONE;
                _dirty = true;
            }
            return;
        }

        if (hit(x, y, 36, Y_VOL + 20, 48, 26)) {
            uint8_t v = Settings::volume();
            Settings::setVolume(v < 5 ? 0 : (uint8_t)(v - 5));
            _dirty = true;
            return;
        }
        if (hit(x, y, SCREEN_WIDTH - 84, Y_VOL + 20, 48, 26)) {
            uint8_t v = Settings::volume();
            Settings::setVolume(v > 95 ? 100 : (uint8_t)(v + 5));
            _dirty = true;
            return;
        }

        if (hit(x, y, 24, Y_LANG, ROW_W, H_ROW)) {
            Settings::nextLang();
            _dirty = true;
            return;
        }

        int16_t half, sndX, micX;
        testGeom(half, sndX, micX);
        if (hit(x, y, sndX, Y_TEST, half, H_ROW)) {
            Audio::playTest();
            _sndOn = Audio::testActive();
            _dirty = true;
            return;
        }
        if (hit(x, y, micX, Y_TEST, half, H_ROW)) {
            Mic::startTest();
            _micOn = Mic::testActive();
            _micDrawMs = millis();
            _dirty = true;
            return;
        }

        if (hit(x, y, 24, Y_WIFI, ROW_W, H_WIFI)) {
            Settings::setWifiEnabled(!Settings::wifiEnabled());
            _dirty = true;
            return;
        }

        if (Settings::wifiConnected() && hit(x, y, 24, Y_TZ, ROW_W, H_TZ)) {
            Settings::nextTz();
            _dirty = true;
            return;
        }

        if (Settings::wifiEnabled() && hit(x, y, 24, Y_AP, ROW_W, H_AP)) {
            Settings::setApEnabled(!Settings::apActive());
            _dirty = true;
            return;
        }

        if (hit(x, y, 24, Y_FACT, ROW_W, H_FACT)) {
            _confirm = CONFIRM_FACTORY;
            _dirty = true;
            return;
        }
        if (hit(x, y, 24, SCREEN_HEIGHT - 56, 150, 44)) {
            _confirm = CONFIRM_RESTART;
            _dirty = true;
            return;
        }
        if (hit(x, y, 194, SCREEN_HEIGHT - 56, 150, 44)) {
            _confirm = CONFIRM_POWER;
            _dirty = true;
        }
    }

private:
    enum Confirm { CONFIRM_NONE, CONFIRM_RESTART, CONFIRM_POWER, CONFIRM_WIFI, CONFIRM_FACTORY };

    static const int16_t ROW_W = SCREEN_WIDTH - 48;
    static const int16_t Y_VOL = 56;
    static const int16_t H_VOL = 48;
    static const int16_t Y_LANG = 108;
    static const int16_t H_ROW = 36;
    static const int16_t Y_TEST = 148;
    static const int16_t Y_WIFI = 188;
    static const int16_t H_WIFI = 36;
    static const int16_t Y_EXTRA = 228;
    static const int16_t Y_TZ = 252;
    static const int16_t H_TZ = 32;
    static const int16_t Y_AP = 288;
    static const int16_t H_AP = 26;
    static const int16_t Y_FACT = 330;
    static const int16_t H_FACT = 28;

    bool _dirty = true;
    Confirm _confirm = CONFIRM_NONE;
    bool _wifiWasOk = false;
    bool _apWasOn = false;
    bool _sndOn = false;
    bool _micOn = false;
    unsigned long _micDrawMs = 0;
    char _langShown[4] = "";

    static bool hit(uint16_t x, uint16_t y, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
        return x >= (uint16_t)rx && x < (uint16_t)(rx + rw) &&
               y >= (uint16_t)ry && y < (uint16_t)(ry + rh);
    }

    static void testGeom(int16_t &half, int16_t &sndX, int16_t &micX) {
        const int16_t gap = 8;
        half = (int16_t)((ROW_W - gap) / 2);
        sndX = 24;
        micX = (int16_t)(24 + half + gap);
    }

    static const char *testStatus(bool ready, bool active, I18nId idle, I18nId busy) {
        if (!ready) return I18n::t(I18N_NA);
        return I18n::t(active ? busy : idle);
    }

    void renderConfirm() {
        Display::fillScreen(COLOR_BG);
        StatusBar::draw(I18n::t(I18N_SETTINGS));
        const char *title = I18n::t(I18N_RESTART_Q);
        const char *hint = I18n::t(I18N_RESTART_HINT);
        if (_confirm == CONFIRM_POWER) {
            title = I18n::t(I18N_POWER_Q);
            hint = I18n::t(I18N_POWER_HINT);
        } else if (_confirm == CONFIRM_WIFI) {
            title = I18n::t(I18N_WIFI_OK);
            hint = I18n::t(I18N_WIFI_OK_HINT);
        } else if (_confirm == CONFIRM_FACTORY) {
            title = I18n::t(I18N_FACTORY_Q);
            hint = I18n::t(I18N_FACTORY_HINT);
        }
        Display::drawText(24, 120, title, COLOR_MAIN, FONT_TITLE);
        Display::drawText(24, 170, hint, COLOR_MUTED, FONT_UI);
        Display::fillRoundRect(24, 280, 150, 52, 14, COLOR_PINK);
        Display::drawText(70, 294, I18n::t(I18N_YES), COLOR_BG, FONT_UI);
        Display::fillRoundRect(194, 280, 150, 52, 14, COLOR_PANEL);
        Display::drawText(248, 294, I18n::t(I18N_NO), COLOR_FG, FONT_UI);
    }

    void render() {
        if (_confirm != CONFIRM_NONE) {
            renderConfirm();
            return;
        }

        Display::fillScreen(COLOR_BG);
        StatusBar::draw(I18n::t(I18N_SETTINGS));

        Display::fillRoundRect(24, Y_VOL, ROW_W, H_VOL, 14, COLOR_PANEL);
        Display::drawText(40, Y_VOL + 4, I18n::t(I18N_VOLUME), COLOR_MUTED, FONT_UI);
        uint8_t vol = Settings::volume();
        char volBuf[8];
        snprintf(volBuf, sizeof(volBuf), "%u%%", (unsigned)vol);
        Display::drawText(140, Y_VOL + 4, volBuf, COLOR_MAIN, FONT_UI);
        Display::fillRoundRect(36, Y_VOL + 24, 44, 20, 6, COLOR_BG);
        Display::drawText(50, Y_VOL + 26, "-", COLOR_FG, FONT_UI);
        int16_t barW = SCREEN_WIDTH - 200;
        Display::fillRect(88, Y_VOL + 30, barW, 8, COLOR_BG);
        int16_t fill = (int16_t)(barW * vol / 100);
        if (fill > 0) Display::fillRect(88, Y_VOL + 30, fill, 8, COLOR_SECOND);
        Display::fillRoundRect(SCREEN_WIDTH - 80, Y_VOL + 24, 44, 20, 6, COLOR_BG);
        Display::drawText(SCREEN_WIDTH - 64, Y_VOL + 26, "+", COLOR_FG, FONT_UI);

        Display::fillRoundRect(24, Y_LANG, ROW_W, H_ROW, 12, COLOR_PANEL);
        Display::drawText(40, Y_LANG + 8, I18n::t(I18N_LANGUAGE), COLOR_FG, FONT_UI);
        Display::drawText(SCREEN_WIDTH - 130, Y_LANG + 8, Settings::langLabel(), COLOR_GOLD, FONT_UI);

        int16_t half, sndX, micX;
        testGeom(half, sndX, micX);
        bool sndReady = Audio::isReady();
        bool micReady = Mic::isReady();
        Display::fillRoundRect(sndX, Y_TEST, half, H_ROW, 12, COLOR_PANEL);
        Display::drawText(sndX + 10, Y_TEST + 8, I18n::t(I18N_SOUND), COLOR_FG, FONT_UI);
        Display::drawText(sndX + half - 70, Y_TEST + 8,
                         testStatus(sndReady, _sndOn, I18N_PLAY, I18N_PLAYING),
                         !sndReady ? COLOR_MUTED : (_sndOn ? COLOR_SECOND : COLOR_GOLD), FONT_UI);

        Display::fillRoundRect(micX, Y_TEST, half, H_ROW, 12, COLOR_PANEL);
        Display::drawText(micX + 10, Y_TEST + 4, I18n::t(I18N_MIC), COLOR_FG, FONT_UI);
        Display::drawText(micX + half - 78, Y_TEST + 4,
                         testStatus(micReady, _micOn, I18N_TEST, I18N_LISTENING),
                         !micReady ? COLOR_MUTED : (_micOn ? COLOR_SECOND : COLOR_GOLD), FONT_UI);
        int16_t meterX = micX + 10;
        int16_t meterW = half - 20;
        Display::fillRect(meterX, Y_TEST + 26, meterW, 4, COLOR_BG);
        uint8_t lvl = _micOn ? Mic::level() : 0;
        int16_t mf = (int16_t)(meterW * lvl / 100);
        if (mf > 0) Display::fillRect(meterX, Y_TEST + 26, mf, 4, COLOR_SECOND);

        bool on = Settings::wifiEnabled();
        bool ok = Settings::wifiConnected();
        Display::fillRoundRect(24, Y_WIFI, ROW_W, H_WIFI, 12, COLOR_PANEL);
        Display::drawText(40, Y_WIFI + 8, I18n::t(I18N_WIFI), COLOR_FG, FONT_UI);
        Display::drawText(SCREEN_WIDTH - 130, Y_WIFI + 8,
                         !on ? I18n::t(I18N_OFF) : (ok ? I18n::t(I18N_CONNECTED) : I18n::t(I18N_NOT_CONNECTED)),
                         !on ? COLOR_MUTED : (ok ? COLOR_SECOND : COLOR_ERROR), FONT_UI);

        if (!on) {
            Display::drawText(24, Y_EXTRA, I18n::t(I18N_RADIO_OFF), COLOR_MUTED, FONT_UI);
        } else if (ok) {
            String ssid = WiFi.SSID();
            Display::drawText(24, Y_EXTRA, ssid.c_str(), COLOR_MAIN, FONT_UI);
            Display::fillRoundRect(24, Y_TZ, ROW_W, H_TZ, 10, COLOR_PANEL);
            Display::drawText(40, Y_TZ + 6, Settings::tzId(), COLOR_MAIN, FONT_UI);
            Display::drawText(SCREEN_WIDTH - 140, Y_TZ + 6,
                             Settings::ntpSynced() ? I18n::t(I18N_NTP_OK) : I18n::t(I18N_NTP_WAIT),
                             Settings::ntpSynced() ? COLOR_SECOND : COLOR_MUTED, FONT_UI);
        } else if (Settings::apActive()) {
            Display::drawText(24, Y_EXTRA, I18n::t(I18N_SETUP_HOTSPOT), COLOR_ERROR, FONT_UI);
            Display::drawText(24, Y_EXTRA + 18, Settings::apSsid(), COLOR_MAIN, FONT_UI);
            char url[48];
            snprintf(url, sizeof(url), "%s:%d/setup",
                     WiFi.softAPIP().toString().c_str(), WEB_CONSOLE_PORT);
            Display::drawText(24, Y_EXTRA + 36, url, COLOR_SECOND, 1);
        } else {
            Display::drawText(24, Y_EXTRA, I18n::t(I18N_HOTSPOT_HINT), COLOR_MUTED, FONT_UI);
        }

        if (on) {
            Display::fillRoundRect(24, Y_AP, ROW_W, H_AP, 8, COLOR_PANEL);
            Display::drawText(40, Y_AP + 4,
                             Settings::apActive() ? I18n::t(I18N_STOP_HOTSPOT) : I18n::t(I18N_START_HOTSPOT),
                             COLOR_SECOND, FONT_UI);
        }

        Display::fillRoundRect(24, Y_FACT, ROW_W, H_FACT, 8, COLOR_PANEL);
        Display::drawText(40, Y_FACT + 4, I18n::t(I18N_FACTORY), COLOR_ERROR, FONT_UI);
        Display::fillRoundRect(24, SCREEN_HEIGHT - 56, 150, 44, 14, COLOR_PANEL);
        Display::drawText(52, SCREEN_HEIGHT - 44, I18n::t(I18N_RESTART), COLOR_FG, FONT_UI);
        Display::fillRoundRect(194, SCREEN_HEIGHT - 56, 150, 44, 14, COLOR_PANEL);
        Display::drawText(222, SCREEN_HEIGHT - 44, I18n::t(I18N_POWER_OFF), COLOR_ERROR, FONT_UI);
    }
};

static SettingsApp settingsApp;

void registerSettingsApp() {
    g_appManager.registerApp(&settingsApp);
}
