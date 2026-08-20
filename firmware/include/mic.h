#ifndef ATHENAOS_MIC_H
#define ATHENAOS_MIC_H

#include <Arduino.h>
#include "boards/board.h"

class Mic {
public:
    static bool begin();
    static bool isReady() { return _ready; }
    static void startTest(uint16_t ms = 1800);
    static bool testActive();
    static uint8_t level();
    static void poll();

private:
    static bool _ready;
    static uint8_t _level;
    static unsigned long _testUntil;
};

#endif
