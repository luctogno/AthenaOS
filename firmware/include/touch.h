#ifndef ATHENAOS_TOUCH_H
#define ATHENAOS_TOUCH_H

#include <Arduino.h>
#include "boards/board.h"

class Touch {
public:
    static bool begin();
    static bool read(uint16_t *x, uint16_t *y);
    static bool isReady() { return _ready; }

private:
    static bool _ready;
    static uint8_t _addr;
};

#endif
