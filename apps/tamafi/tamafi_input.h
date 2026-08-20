#ifndef TAMAFI_INPUT_H
#define TAMAFI_INPUT_H

#include <Arduino.h>

struct TamafiInputEvent {
    bool up = false;
    bool ok = false;
    bool down = false;
    bool r1 = false;
    bool r2 = false;
    bool r3 = false;
    bool tap = false;
    int16_t tapX = 0;
    int16_t tapY = 0;
};

class TamafiInput {
public:
    static bool begin();
    static TamafiInputEvent poll();
};

#endif
