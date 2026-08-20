#ifndef ATHENAOS_AUDIO_H
#define ATHENAOS_AUDIO_H

#include <Arduino.h>
#include "boards/board.h"

class Audio {
public:
    static bool begin();
    static bool isReady() { return _ready; }

private:
    static bool _ready;
};

#endif
