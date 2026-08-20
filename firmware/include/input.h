#ifndef ATHENAOS_INPUT_H
#define ATHENAOS_INPUT_H

#include <Arduino.h>
#include "boards/board.h"

struct InputEvent {
    bool btn1 = false;
    bool btn2 = false;
    bool btn3 = false;
    bool tap = false;
    int16_t tapX = 0;
    int16_t tapY = 0;
};

class Input {
public:
    static bool begin();
    static InputEvent poll();
};

#endif
