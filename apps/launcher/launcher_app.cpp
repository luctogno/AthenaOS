#include "app_manager.h"
#include "display.h"
#include "boards/board.h"
#include "app_icon.h"
#include <string.h>
#include <stdio.h>

class LauncherApp : public App {
public:
    AppManifest getManifest() override {
        return {"launcher", "Launcher", "1.0.0", "AthenaOS", true, APP_ICON_DEFAULT, nullptr, 0, 0};
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

    void onTouchDown(uint16_t x, uint16_t y) override {
        int n = visibleCount();
        for (int i = 0; i < n; i++) {
            int16_t tx, ty, tw, th;
            tileRect(i, n, tx, ty, tw, th);
            if (x >= (uint16_t)tx && x < (uint16_t)(tx + tw) &&
                y >= (uint16_t)ty && y < (uint16_t)(ty + th)) {
                App *app = visibleApp(i);
                if (app) {
                    AppManifest m = app->getManifest();
                    g_appManager.switchToAppById(m.id);
                }
                return;
            }
        }
    }

    void update() override {}

private:
    bool _dirty = true;

    static bool isLauncher(App *app) {
        if (!app) return true;
        AppManifest m = app->getManifest();
        return m.id && strcmp(m.id, "launcher") == 0;
    }

    static int visibleCount() {
        int n = 0;
        for (int i = 0; i < g_appManager.getAppCount(); i++) {
            if (!isLauncher(g_appManager.getApp(i))) n++;
        }
        return n;
    }

    static App *visibleApp(int visIndex) {
        int n = 0;
        for (int i = 0; i < g_appManager.getAppCount(); i++) {
            App *a = g_appManager.getApp(i);
            if (isLauncher(a)) continue;
            if (n == visIndex) return a;
            n++;
        }
        return nullptr;
    }

    static void tileRect(int index, int count, int16_t &x, int16_t &y, int16_t &w, int16_t &h) {
        const int16_t pad = 24;
        const int16_t top = 88;
        w = SCREEN_WIDTH - pad * 2;
        if (count <= 1) {
            h = 220;
            x = pad;
            y = top;
            return;
        }
        h = 96;
        x = pad;
        y = top + index * (h + 16);
    }

    void render() {
        Display::fillScreen(COLOR_BG);
        Display::fillRect(0, 0, SCREEN_WIDTH, 64, COLOR_PANEL);
        Display::drawText(24, 22, "AthenaOS", COLOR_GOLD, 2);

        int n = visibleCount();
        if (n == 0) {
            Display::drawText(32, 180, "No apps", COLOR_MUTED, 2);
            return;
        }

        for (int i = 0; i < n; i++) {
            App *app = visibleApp(i);
            if (!app) continue;
            AppManifest m = app->getManifest();
            int16_t x, y, w, h;
            tileRect(i, n, x, y, w, h);
            Display::fillRoundRect(x, y, w, h, 18, COLOR_PANEL);
            drawAppIcon(x + 48, y + h / 2, 28, m.icon, m.iconBitmap, m.iconW, m.iconH);
            Display::drawText(x + 92, y + h / 2 - 22, m.name, COLOR_FG, 2);
            Display::drawText(x + 92, y + h / 2 + 8, "Tap to open", COLOR_MUTED, 1);
        }

        char holdHint[32];
        snprintf(holdHint, sizeof(holdHint), "Hold BOOT %lus to go home",
                 (unsigned long)(HOME_HOLD_MS / 1000));
        Display::drawText(24, SCREEN_HEIGHT - 36, holdHint, COLOR_MUTED, 1);
    }
};

static LauncherApp launcherApp;

void registerLauncherApp() {
    g_appManager.registerApp(&launcherApp);
}
