#ifndef ATHENAOS_ES8311_H
#define ATHENAOS_ES8311_H

#include <Arduino.h>
#include "boards/board.h"

class Es8311 {
public:
    static bool begin();
    static bool isReady() { return _ready; }
    static void setVolume(uint8_t percent);
    static void setPa(bool on);
    static void setMute(bool mute);
    static bool writeStereo(const int16_t *frames, size_t count, uint32_t timeoutMs);
    static size_t readStereo(int16_t *frames, size_t count, uint32_t timeoutMs);

private:
    static bool _ready;
    static bool probe();
    static bool writeReg(uint8_t reg, uint8_t val);
    static bool readReg(uint8_t reg, uint8_t &val);
    static bool initI2s();
    static bool initCodec();
    static void enableMicLdo();
};

#endif
