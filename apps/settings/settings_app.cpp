#include "app_manager.h"
#include "display.h"
#include "boards/board.h"
#include "icon.h"
#include "settings.h"
#include "status_bar.h"
#include "power.h"
#include "i18n.h"

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
                else Power::restart();
            } else if (hit(x, y, 194, 280, 150, 52)) {
                _confirm = CONFIRM_NONE;
                _dirty = true;
            }
            return;
        }

        if (hit(x, y, SCREEN_WIDTH - 90, 64, 66, 32)) {
            Settings::toggleLang();
            _dirty = true;
            return;
        }

        if (hit(x, y, 36, 90, 48, 44)) {
            uint8_t v = Settings::volume();
            Settings::setVolume(v < 5 ? 0 : (uint8_t)(v - 5));
            _dirty = true;
            return;
        }
        if (hit(x, y, SCREEN_WIDTH - 84, 90, 48, 44)) {
            uint8_t v = Settings::volume();
            Settings::setVolume(v > 95 ? 100 : (uint8_t)(v + 5));
            _dirty = true;
            return;
        }

        if (hit(x, y, 24, 148, SCREEN_WIDTH - 48, 52)) {
            Settings::setWifiEnabled(!Settings::wifiEnabled());
            _dirty = true;
            return;
        }

        if (Settings::wifiConnected() && hit(x, y, 24, 276, SCREEN_WIDTH - 48, 44)) {
            Settings::nextTz();
            _dirty = true;
            return;
        }

        if (Settings::wifiEnabled() && hit(x, y, 24, 336, SCREEN_WIDTH - 48, 36)) {
            Settings::setApEnabled(!Settings::apActive());
            _dirty = true;
            return;
        }

        if (hit(x, y, 24, SCREEN_HEIGHT - 72, 150, 48)) {
            _confirm = CONFIRM_RESTART;
            _dirty = true;
            return;
        }
        if (hit(x, y, 194, SCREEN_HEIGHT - 72, 150, 48)) {
            _confirm = CONFIRM_POWER;
            _dirty = true;
        }
    }

private:
    enum Confirm { CONFIRM_NONE, CONFIRM_RESTART, CONFIRM_POWER, CONFIRM_WIFI };

    bool _dirty = true;
    Confirm _confirm = CONFIRM_NONE;
    bool _wifiWasOk = false;
    bool _apWasOn = false;
    char _langShown[4] = "";

    static bool hit(uint16_t x, uint16_t y, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
        return x >= (uint16_t)rx && x < (uint16_t)(rx + rw) &&
               y >= (uint16_t)ry && y < (uint16_t)(ry + rh);
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

        Display::fillRoundRect(24, 64, SCREEN_WIDTH - 48, 72, 16, COLOR_PANEL);
        Display::drawText(40, 74, I18n::t(I18N_VOLUME), COLOR_MUTED, FONT_UI);
        uint8_t vol = Settings::volume();
        char volBuf[8];
        snprintf(volBuf, sizeof(volBuf), "%u%%", (unsigned)vol);
        Display::drawText(140, 74, volBuf, COLOR_MAIN, FONT_UI);
        Display::drawText(SCREEN_WIDTH - 72, 74, Settings::langIsIt() ? "IT" : "EN", COLOR_GOLD, FONT_UI);
        Display::fillRoundRect(36, 98, 48, 28, 8, COLOR_BG);
        Display::drawText(50, 102, "-", COLOR_FG, FONT_UI);
        int16_t barW = SCREEN_WIDTH - 200;
        Display::fillRect(96, 108, barW, 10, COLOR_BG);
        int16_t fill = (int16_t)(barW * vol / 100);
        if (fill > 0) Display::fillRect(96, 108, fill, 10, COLOR_SECOND);
        Display::fillRoundRect(SCREEN_WIDTH - 84, 98, 48, 28, 8, COLOR_BG);
        Display::drawText(SCREEN_WIDTH - 66, 102, "+", COLOR_FG, FONT_UI);

        bool on = Settings::wifiEnabled();
        bool ok = Settings::wifiConnected();
        Display::fillRoundRect(24, 148, SCREEN_WIDTH - 48, 52, 16, COLOR_PANEL);
        Display::drawText(40, 164, I18n::t(I18N_WIFI), COLOR_FG, FONT_UI);
        Display::drawText(SCREEN_WIDTH - 130, 164,
                         !on ? I18n::t(I18N_OFF) : (ok ? I18n::t(I18N_CONNECTED) : I18n::t(I18N_NOT_CONNECTED)),
                         !on ? COLOR_MUTED : (ok ? COLOR_SECOND : COLOR_ERROR), FONT_UI);

        if (!on) {
            Display::drawText(24, 220, I18n::t(I18N_RADIO_OFF), COLOR_MUTED, FONT_UI);
        } else if (ok) {
            String ssid = WiFi.SSID();
            Display::drawText(24, 216, I18n::t(I18N_NETWORK), COLOR_MUTED, FONT_UI);
            Display::drawText(24, 240, ssid.c_str(), COLOR_MAIN, FONT_UI);
            Display::fillRoundRect(24, 276, SCREEN_WIDTH - 48, 44, 12, COLOR_PANEL);
            Display::drawText(40, 288, Settings::tzId(), COLOR_MAIN, FONT_UI);
            Display::drawText(SCREEN_WIDTH - 140, 288,
                             Settings::ntpSynced() ? I18n::t(I18N_NTP_OK) : I18n::t(I18N_NTP_WAIT),
                             Settings::ntpSynced() ? COLOR_SECOND : COLOR_MUTED, FONT_UI);
        } else if (Settings::apActive()) {
            Display::drawText(24, 212, I18n::t(I18N_SETUP_HOTSPOT), COLOR_ERROR, FONT_UI);
            Display::drawText(24, 236, Settings::apSsid(), COLOR_MAIN, FONT_UI);
            Display::drawText(24, 260, I18n::t(I18N_JOIN_WIFI), COLOR_FG, 1);
            Display::drawText(24, 278, I18n::t(I18N_OPEN_PHONE), COLOR_FG, 1);
            char url[48];
            snprintf(url, sizeof(url), "%s:%d/setup",
                     WiFi.softAPIP().toString().c_str(), WEB_CONSOLE_PORT);
            Display::drawText(24, 296, url, COLOR_SECOND, 1);
            Display::drawText(24, 314, I18n::t(I18N_PICK_WIFI), COLOR_FG, 1);
        } else {
            Display::drawText(24, 212, I18n::t(I18N_SETUP_HOTSPOT), COLOR_MUTED, FONT_UI);
            Display::drawText(24, 236, I18n::t(I18N_HOTSPOT_HINT), COLOR_FG, FONT_UI);
        }

        if (on) {
            Display::fillRoundRect(24, 336, SCREEN_WIDTH - 48, 36, 10, COLOR_PANEL);
            Display::drawText(40, 344,
                             Settings::apActive() ? I18n::t(I18N_STOP_HOTSPOT) : I18n::t(I18N_START_HOTSPOT),
                             COLOR_SECOND, FONT_UI);
        }

        Display::fillRoundRect(24, SCREEN_HEIGHT - 72, 150, 48, 14, COLOR_PANEL);
        Display::drawText(52, SCREEN_HEIGHT - 58, I18n::t(I18N_RESTART), COLOR_FG, FONT_UI);
        Display::fillRoundRect(194, SCREEN_HEIGHT - 72, 150, 48, 14, COLOR_PANEL);
        Display::drawText(222, SCREEN_HEIGHT - 58, I18n::t(I18N_POWER_OFF), COLOR_ERROR, FONT_UI);
    }
};

static SettingsApp settingsApp;

void registerSettingsApp() {
    g_appManager.registerApp(&settingsApp);
}
