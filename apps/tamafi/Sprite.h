#ifndef TAMAFI_SPRITE_H
#define TAMAFI_SPRITE_H

#include <Arduino.h>
#include "config.h"
#include "display.h"

#define TFT_BLACK     0x0000
#define TFT_WHITE     0xFFFF
#define TFT_RED       0xF800
#define TFT_GREEN     0x07E0
#define TFT_BLUE      0x001F
#define TFT_CYAN      0x07FF
#define TFT_MAGENTA   0xF81F
#define TFT_YELLOW    0xFFE0
#define TFT_DARKGREY  0x7BEF
#define MAT_RED       0xF206  /* Material Red 500    #F44336 */
#define MAT_YELLOW    0xFF47  /* Material Yellow 500 #FFEB3B */
#define MAT_GREEN     0x4D6A  /* Material Green 500  #4CAF50 */
#define STATS_GRAY    0x5268  /* STATS header fill   #534E47 */
#define STATS_GRAY_HI 0x6B4C  /* STATS header bevel  #716A63 */

class Sprite : public Print {
public:
    Sprite(void * = nullptr) {}

    void setColorDepth(int) {}
    void setSwapBytes(bool v) { _swap = v; }
    bool createSprite(int16_t w, int16_t h);

    void fillSprite(uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t c);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t c);
    void drawPixel(int16_t x, int16_t y, uint16_t c);

    void setTextColor(uint16_t c) { _text = c; }
    void setTextSize(uint8_t s) { _textSize = s < 1 ? 1 : s; }
    void setCursor(int16_t x, int16_t y) { _cx = x; _cy = y; }

    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data);
    void pushImageSrc(int16_t x, int16_t y, int16_t dw, int16_t dh,
                      const uint16_t *data, int16_t sw, int16_t sh);
    void pushToSprite(Sprite *dst, int16_t x, int16_t y, uint16_t trans);
    void pushToSpriteScaled(Sprite *dst, int16_t x, int16_t y, uint16_t trans, float scale);
    void pushSprite(int16_t x, int16_t y);

    uint16_t *buffer() { return _buf; }
    int16_t width() const { return _w; }
    int16_t height() const { return _h; }

    using Print::write;
    size_t write(uint8_t c) override;

private:
    uint16_t *_buf = nullptr;
    bool _swap = false;
    int16_t _w = 0;
    int16_t _h = 0;
    int16_t _cx = 0;
    int16_t _cy = 0;
    uint16_t _text = TFT_WHITE;
    uint8_t _textSize = 1;

    void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint8_t size);
};

#endif
