#include "app_manager.h"
#include "display.h"
#include "boards/board.h"
#include "status_bar.h"
#include <stdio.h>

// AthenaOS shell for upstream 78/xiaozhi-esp32.
// Do not include third_party/xiaozhi-esp32 headers here: that tree is ESP-IDF
// (app_main + LVGL + ESP-SR) and must stay a git submodule, unmodified.

class XiaozhiApp : public App {
public:
    AppManifest getManifest() override {
        return {"xiaozhi", "Xiaozhi", "1.0.0", "78/xiaozhi-esp32", true, APP_ICON_MIC, nullptr, 0, 0};
    }

    void start() override {
        state = STATE_RUNNING;
        _dirty = true;
    }

    void resume() override {
        state = STATE_RUNNING;
        _dirty = true;
    }

    void draw() override {
        if (!_dirty) return;
        _dirty = false;
        render();
    }

    void onTouchDown(uint16_t, uint16_t) override {}

    void update() override {}

private:
    bool _dirty = true;

    void render() {
        Display::fillScreen(COLOR_BG);
        StatusBar::draw("Xiaozhi");

        Display::fillRoundRect(24, 88, SCREEN_WIDTH - 48, 120, 16, COLOR_PANEL);
        Display::drawText(40, 108, "AI voice / MCP", COLOR_FG, 2);
        Display::drawText(40, 148, "submodule, not IDF build", COLOR_MUTED, 1);

        Display::drawText(24, 228, "Upstream", COLOR_MUTED, 1);
        Display::drawText(24, 248, "third_party/xiaozhi-esp32", COLOR_FG, 1);

        Display::drawText(24, 280, "IDF board", COLOR_MUTED, 1);
#if WAVESHARE_AMOLED_V2
        Display::drawText(24, 300, "amoled-1.8-v2", COLOR_FG, 1);
#else
        Display::drawText(24, 300, "amoled-1.8", COLOR_FG, 1);
#endif

        Display::drawText(24, 340, "Voice/ASR/TTS stay in ESP-IDF.", COLOR_MUTED, 1);
        Display::drawText(24, 360, "This app is the AthenaOS host.", COLOR_MUTED, 1);

        char holdHint[32];
        snprintf(holdHint, sizeof(holdHint), "Hold BOOT %lus to go home",
                 (unsigned long)(HOME_HOLD_MS / 1000));
        Display::drawText(24, SCREEN_HEIGHT - 36, holdHint, COLOR_MUTED, 1);
    }
};

static XiaozhiApp xiaozhiApp;

void registerXiaozhiApp() {
    g_appManager.registerApp(&xiaozhiApp);
}
