#include "app_manager.h"
#include "display.h"
#include "boards/board.h"
#include "app_icon.h"
#include "settings.h"
#include "status_bar.h"
#include "power.h"

#include <WiFi.h>
#include <stdio.h>

class SettingsApp : public App {
public:
    AppManifest getManifest() override {
        return {"settings", "Settings", "1.0.0", "AthenaOS", true, APP_ICON_GEAR, nullptr, 0, 0};
    }

    void start() override {
        state = STATE_RUNNING;
        _confirm = CONFIRM_NONE;
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

        if (!Settings::wifiConnected() && Settings::wifiEnabled() &&
            hit(x, y, 24, 336, SCREEN_WIDTH - 48, 36)) {
            Settings::startAp();
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

    static bool hit(uint16_t x, uint16_t y, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
        return x >= (uint16_t)rx && x < (uint16_t)(rx + rw) &&
               y >= (uint16_t)ry && y < (uint16_t)(ry + rh);
    }

    void renderConfirm() {
        Display::fillScreen(COLOR_BG);
        StatusBar::draw("Settings");
        const char *title = "Restart?";
        const char *hint = "Device will reboot";
        if (_confirm == CONFIRM_POWER) {
            title = "Power off?";
            hint = "Hold PWR to turn on";
        } else if (_confirm == CONFIRM_WIFI) {
            title = "WiFi connected";
            hint = "Restart to apply?";
        }
        Display::drawText(24, 120, title, COLOR_MAIN, FONT_TITLE);
        Display::drawText(24, 170, hint, COLOR_MUTED, FONT_UI);
        Display::fillRoundRect(24, 280, 150, 52, 14, COLOR_SECOND);
        Display::drawText(70, 294, "Yes", COLOR_BG, FONT_UI);
        Display::fillRoundRect(194, 280, 150, 52, 14, COLOR_PANEL);
        Display::drawText(248, 294, "No", COLOR_FG, FONT_UI);
    }

    void render() {
        if (_confirm != CONFIRM_NONE) {
            renderConfirm();
            return;
        }

        Display::fillScreen(COLOR_BG);
        StatusBar::draw("Settings");

        Display::fillRoundRect(24, 64, SCREEN_WIDTH - 48, 72, 16, COLOR_PANEL);
        Display::drawText(40, 74, "Volume", COLOR_MUTED, FONT_UI);
        uint8_t vol = Settings::volume();
        char volBuf[8];
        snprintf(volBuf, sizeof(volBuf), "%u%%", (unsigned)vol);
        Display::drawText(140, 74, volBuf, COLOR_MAIN, FONT_UI);
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
        Display::drawText(40, 164, "WiFi", COLOR_FG, FONT_UI);
        Display::drawText(SCREEN_WIDTH - 130, 164,
                         !on ? "Off" : (ok ? "Connected" : "Not connected"),
                         !on ? COLOR_MUTED : (ok ? COLOR_SECOND : COLOR_ERROR), FONT_UI);

        if (!on) {
            Display::drawText(24, 220, "Radio off", COLOR_MUTED, FONT_UI);
        } else if (ok) {
            String ssid = WiFi.SSID();
            Display::drawText(24, 216, "Network", COLOR_MUTED, FONT_UI);
            Display::drawText(24, 240, ssid.c_str(), COLOR_MAIN, FONT_UI);
            Display::fillRoundRect(24, 276, SCREEN_WIDTH - 48, 44, 12, COLOR_PANEL);
            Display::drawText(40, 288, Settings::tzId(), COLOR_MAIN, FONT_UI);
            Display::drawText(SCREEN_WIDTH - 140, 288,
                             Settings::ntpSynced() ? "NTP ok" : "NTP...",
                             Settings::ntpSynced() ? COLOR_SECOND : COLOR_MUTED, FONT_UI);
        } else {
            Display::drawText(24, 212, "Setup hotspot", COLOR_ERROR, FONT_UI);
            Display::drawText(24, 236, Settings::apSsid(), COLOR_MAIN, FONT_UI);
            Display::drawText(24, 260, "1. Join this WiFi", COLOR_FG, 1);
            Display::drawText(24, 278, "2. Open on phone:", COLOR_FG, 1);
            char url[48];
            snprintf(url, sizeof(url), "%s:%d/setup",
                     WiFi.softAPIP().toString().c_str(), WEB_CONSOLE_PORT);
            Display::drawText(24, 296, url, COLOR_SECOND, 1);
            Display::drawText(24, 314, "3. Pick home WiFi", COLOR_FG, 1);
            Display::fillRoundRect(24, 336, SCREEN_WIDTH - 48, 36, 10, COLOR_PANEL);
            Display::drawText(40, 344,
                             Settings::apActive() ? "Hotspot on" : "Start hotspot",
                             COLOR_SECOND, FONT_UI);
        }

        Display::fillRoundRect(24, SCREEN_HEIGHT - 72, 150, 48, 14, COLOR_PANEL);
        Display::drawText(52, SCREEN_HEIGHT - 58, "Restart", COLOR_FG, FONT_UI);
        Display::fillRoundRect(194, SCREEN_HEIGHT - 72, 150, 48, 14, COLOR_PANEL);
        Display::drawText(222, SCREEN_HEIGHT - 58, "Power off", COLOR_ERROR, FONT_UI);
    }
};

static SettingsApp settingsApp;

void registerSettingsApp() {
    g_appManager.registerApp(&settingsApp);
}
