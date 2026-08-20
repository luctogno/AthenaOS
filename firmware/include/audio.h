#ifndef ATHENAOS_AUDIO_H
#define ATHENAOS_AUDIO_H

#include <Arduino.h>
#include "boards/board.h"

class Audio {
public:
    static bool begin();
    static bool isReady() { return _ready; }
    static void setVolume(uint8_t percent);
    static uint8_t volume() { return _volume; }

private:
    static bool _ready;
    static uint8_t _volume;
};

#endif
