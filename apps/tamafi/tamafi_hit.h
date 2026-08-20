#ifndef TAMAFI_HIT_H
#define TAMAFI_HIT_H

#include <Arduino.h>
#include "config.h"

namespace TamafiHit {
    bool hitStats(int16_t x, int16_t y);
    bool hitRestBtn(int16_t x, int16_t y);
    bool hitPlayBtn(int16_t x, int16_t y);
}

#endif
