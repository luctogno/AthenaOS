#include "splash.h"
#include "display.h"
#include "boards/board.h"

static unsigned long splashStart = 0;
static const unsigned long SPLASH_MS = 1800;
static uint8_t lastPct = 0;

static const int16_t BAR_W = 240;
static const int16_t BAR_H = 12;
static int16_t barX = 0;
static int16_t barY = 0;

static void drawOwl(int16_t cx, int16_t cy) {
    Display::fillCircle(cx, cy + 6, 62, 0x1C4C);
    Display::fillCircle(cx, cy, 52, 0x0B2A);
    Display::fillCircle(cx - 18, cy - 6, 14, COLOR_FG);
    Display::fillCircle(cx + 18, cy - 6, 14, COLOR_FG);
    Display::fillCircle(cx - 16, cy - 5, 6, 0x0000);
    Display::fillCircle(cx + 20, cy - 5, 6, 0x0000);
    Display::fillTriangle(cx, cy + 4, cx - 10, cy + 18, cx + 10, cy + 18, COLOR_GOLD);
    Display::fillCircle(cx, cy + 36, 10, 0x1C4C);
}

static void drawBarFill(uint8_t pct) {
    int16_t innerW = BAR_W - 4;
    int16_t fill = (int16_t)(innerW * pct / 100);
    if (fill < 0) fill = 0;
    if (fill > innerW) fill = innerW;
    if (fill > 0) {
        Display::fillRect(barX + 2, barY + 2, fill, BAR_H - 4, COLOR_ACCENT);
    }
}

void Splash::begin() {
    splashStart = millis();
    lastPct = 0;

    Display::fillScreen(COLOR_BG);
    const int16_t cx = SCREEN_WIDTH / 2;
    drawOwl(cx, 128);
    Display::drawText(cx - 72, 248, "AthenaOS", COLOR_MAIN, FONT_TITLE);
    Display::drawText(cx - 58, 288, "for Athena", COLOR_FG, FONT_UI);

    barX = (SCREEN_WIDTH - BAR_W) / 2;
    barY = 360;
    Display::drawRect(barX, barY, BAR_W, BAR_H, COLOR_ACCENT);
    Display::fillRect(barX + 2, barY + 2, BAR_W - 4, BAR_H - 4, COLOR_BG);
}

bool Splash::update() {
    unsigned long elapsed = millis() - splashStart;
    uint8_t pct = (elapsed >= SPLASH_MS) ? 100 : (uint8_t)(elapsed * 100 / SPLASH_MS);
    if (pct > lastPct) {
        lastPct = pct;
        drawBarFill(pct);
    }
    return elapsed >= SPLASH_MS;
}
