#include "splash.h"
#include "audio.h"
#include "display.h"
#include "boards/board.h"
#include "splash_embed.h"

#include <JPEGDEC.h>

static unsigned long splashStart = 0;
static const unsigned long SPLASH_MS = 2200;
static uint8_t lastPct = 0;
static bool beeped = false;
static const int16_t BAR_H = 6;

static int jpegBlit(JPEGDRAW *draw) {
    Display::blit(draw->x, draw->y, draw->pPixels, draw->iWidth, draw->iHeight);
    return 1;
}

static void drawBarFill(uint8_t pct) {
    int16_t prev = (int16_t)((int32_t)SCREEN_WIDTH * lastPct / 100);
    int16_t fill = (int16_t)((int32_t)SCREEN_WIDTH * pct / 100);
    if (fill > prev) {
        Display::fillRect(prev, SCREEN_HEIGHT - BAR_H, fill - prev, BAR_H, COLOR_PINK);
    }
}

void Splash::begin() {
    splashStart = millis();
    lastPct = 0;
    beeped = false;

    Display::fillScreen(COLOR_BG);
    Display::fillRect(0, SCREEN_HEIGHT - BAR_H, SCREEN_WIDTH, BAR_H, COLOR_PANEL);

    JPEGDEC *jpg = new JPEGDEC();
    if (jpg && jpg->openFLASH(SPLASH_JPG, (int)SPLASH_JPG_LEN, jpegBlit)) {
        jpg->setPixelType(RGB565_LITTLE_ENDIAN);
        int16_t x = (SCREEN_WIDTH - jpg->getWidth()) / 2;
        int16_t y = (SCREEN_HEIGHT - BAR_H - jpg->getHeight()) / 2;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        jpg->decode(x, y, 0);
        jpg->close();
    }
    delete jpg;
}

bool Splash::update() {
    unsigned long elapsed = millis() - splashStart;
    uint8_t pct = (elapsed >= SPLASH_MS) ? 100 : (uint8_t)(elapsed * 100 / SPLASH_MS);
    if (pct > lastPct) {
        drawBarFill(pct);
        lastPct = pct;
    }
    if (!beeped && pct >= 90) {
        beeped = true;
        Audio::playTest(200);
    }
    return elapsed >= SPLASH_MS;
}
