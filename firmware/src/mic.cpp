#include "mic.h"

bool Mic::_ready = false;
unsigned long Mic::_testUntil = 0;

void Mic::startTest(uint16_t ms) {
    if (!_ready) {
        DEBUG_PRINTLN("[Mic] startTest skipped (not ready)");
        return;
    }
    if (ms < 200) ms = 200;
    _testUntil = millis() + ms;
    DEBUG_PRINTLN("[Mic] startTest (stub, no capture yet)");
}

bool Mic::testActive() {
    return _ready && (long)(millis() - _testUntil) < 0;
}

uint8_t Mic::level() {
    return 0;
}

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
