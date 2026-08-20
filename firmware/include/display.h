#ifndef ATHENAOS_DISPLAY_H
#define ATHENAOS_DISPLAY_H

#include <Arduino.h>
#include "boards/board.h"

enum UiFont {
    FONT_DEFAULT = 0,
    FONT_UI,
    FONT_TITLE
};

class Display {
public:
    static bool begin();
    static void setBrightness(uint8_t level);
    static void fillScreen(uint16_t color);
    static void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    static void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    static void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    static void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    static void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    static void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    static void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    static void blit(int16_t x, int16_t y, const uint16_t *buf, int16_t w, int16_t h);
    static void drawText(int16_t x, int16_t y, const char *text, uint16_t color);
    static void drawText(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t size);
    static void drawText(int16_t x, int16_t y, const char *text, uint16_t color, UiFont font);
};

#endif
