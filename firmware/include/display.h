#ifndef ATHENAOS_DISPLAY_H
#define ATHENAOS_DISPLAY_H

#include <Arduino.h>
#include "boards/board.h"

#define COLOR_BG      0x0861
#define COLOR_FG      0xFFFF
#define COLOR_ACCENT  0x07FD
#define COLOR_GOLD    0xFE60
#define COLOR_PANEL   0x19C8
#define COLOR_MUTED   0x8410
#define COLOR_ERROR   0xF800

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
};

#endif
