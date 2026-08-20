#ifndef ATHENAOS_PEDOMETER_H
#define ATHENAOS_PEDOMETER_H

#include <Arduino.h>
#include "boards/board.h"

class Pedometer {
public:
    static bool begin();
    static void poll();
    static uint32_t total() { return _total; }
    static bool imuReady();

private:
    static uint32_t _total;
    static bool _armed;
    static float _filt;
    static unsigned long _lastStepMs;
    static unsigned long _lastPollMs;
};

#endif
