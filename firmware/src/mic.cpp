#include "mic.h"

bool Mic::_ready = false;

#if HAS_MIC

bool Mic::begin() {
    DEBUG_PRINTF("[Mic] stub ES8311 ADC I2S DOUT=%d (no capture yet)\n", I2S_DOUT_PIN);
    _ready = true;
    return true;
}

#else

bool Mic::begin() {
    DEBUG_PRINTLN("[Mic] HAS_MIC=0");
    _ready = false;
    return false;
}

#endif
