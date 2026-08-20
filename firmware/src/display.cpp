#include "display.h"

#if HAS_DISPLAY
#include <Arduino_GFX_Library.h>
#include "ui_font.h"

static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    TFT_PIN_CS, TFT_PIN_SCLK, TFT_PIN_D0, TFT_PIN_D1, TFT_PIN_D2, TFT_PIN_D3);

#if WAVESHARE_AMOLED_V2
static Arduino_OLED *gfx = new Arduino_CO5300(bus, GFX_NOT_DEFINED, 0, TFT_WIDTH, TFT_HEIGHT);
#else
static Arduino_OLED *gfx = new Arduino_SH8601(bus, GFX_NOT_DEFINED, 0, TFT_WIDTH, TFT_HEIGHT);
#endif

bool Display::begin() {
    if (!gfx->begin()) {
        DEBUG_PRINTLN("[Display] gfx->begin() failed");
        return false;
    }
    gfx->fillScreen(RGB565_BLACK);
    gfx->setBrightness(180);
    gfx->setTextWrap(false);
    DEBUG_PRINTF("[Display] %dx%d native UI\n", TFT_WIDTH, TFT_HEIGHT);
    return true;
}

void Display::setBrightness(uint8_t level) {
    gfx->setBrightness(level);
}

void Display::fillScreen(uint16_t color) {
    gfx->fillScreen(color);
}

void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    gfx->fillRect(x, y, w, h, color);
}

void Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    gfx->drawRect(x, y, w, h, color);
}

void Display::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Do not use gfx->fillRoundRect: Adafruit fillCircleHelper leaves a 1px
    // noisy scanline on SH8601/CO5300 QSPI, usually along the bottom of the fill.
    if (w <= 0 || h <= 0) return;
    if (r < 0) r = 0;
    int16_t maxR = ((w < h) ? w : h) / 2;
    if (r > maxR) r = maxR;
    if (r < 1) {
        gfx->fillRect(x, y, w, h, color);
        return;
    }

    gfx->fillRect(x, y + r, w, h - 2 * r, color);
    gfx->fillRect(x + r, y, w - 2 * r, r + 1, color);
    gfx->fillRect(x + r, y + h - r - 1, w - 2 * r, r + 1, color);
    gfx->fillCircle(x + r, y + r, r, color);
    gfx->fillCircle(x + w - r - 1, y + r, r, color);
    gfx->fillCircle(x + r, y + h - r - 1, r, color);
    gfx->fillCircle(x + w - r - 1, y + h - r - 1, r, color);
}

void Display::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    gfx->fillCircle(x, y, r, color);
}

void Display::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    gfx->drawCircle(x, y, r, color);
}

void Display::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    gfx->fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    gfx->drawLine(x0, y0, x1, y1, color);
}

void Display::blit(int16_t x, int16_t y, const uint16_t *buf, int16_t w, int16_t h) {
    gfx->draw16bitRGBBitmap(x, y, (uint16_t *)buf, w, h);
}

void Display::drawText(int16_t x, int16_t y, const char *text, uint16_t color) {
    drawText(x, y, text, color, 2);
}

void Display::drawText(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t size) {
    gfx->setFont(nullptr);
    gfx->setTextColor(color);
    gfx->setTextSize(size < 1 ? 1 : size);
    gfx->setCursor(x, y);
    gfx->print(text);
}

void Display::drawText(int16_t x, int16_t y, const char *text, uint16_t color, UiFont font) {
    if (font == FONT_DEFAULT) {
        drawText(x, y, text, color, 2);
        return;
    }
#if UI_FONT_FAMILY == UI_FONT_GLCD
    drawText(x, y, text, color, font == FONT_TITLE ? 3 : 2);
#else
    gfx->setTextColor(color);
    gfx->setTextSize(1);
    if (font == FONT_TITLE) {
        gfx->setFont(&UI_FONT_TITLE_GFX);
        gfx->setCursor(x, y + UI_FONT_TITLE_BASELINE);
    } else {
        gfx->setFont(&UI_FONT_UI_GFX);
        gfx->setCursor(x, y + UI_FONT_UI_BASELINE);
    }
    gfx->print(text);
    gfx->setFont(nullptr);
    gfx->setTextSize(2);
#endif
}

#else

bool Display::begin() {
    DEBUG_PRINTLN("[Display] HAS_DISPLAY=0");
    return false;
}

void Display::setBrightness(uint8_t) {}
void Display::fillScreen(uint16_t) {}
void Display::fillRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
void Display::drawRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
void Display::fillRoundRect(int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
void Display::fillCircle(int16_t, int16_t, int16_t, uint16_t) {}
void Display::drawCircle(int16_t, int16_t, int16_t, uint16_t) {}
void Display::fillTriangle(int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
void Display::drawLine(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
void Display::blit(int16_t, int16_t, const uint16_t *, int16_t, int16_t) {}
void Display::drawText(int16_t, int16_t, const char *, uint16_t) {}
void Display::drawText(int16_t, int16_t, const char *, uint16_t, uint8_t) {}
void Display::drawText(int16_t, int16_t, const char *, uint16_t, UiFont) {}

#endif
