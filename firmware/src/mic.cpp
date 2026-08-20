#include "mic.h"
#include "es8311.h"

bool Mic::_ready = false;
uint8_t Mic::_level = 0;
unsigned long Mic::_testUntil = 0;

bool Mic::testActive() {
    return _ready && (long)(millis() - _testUntil) < 0;
}

uint8_t Mic::level() {
    return testActive() ? _level : 0;
}

#if HAS_MIC

bool Mic::begin() {
    if (!Es8311::begin()) {
        DEBUG_PRINTLN("[Mic] ES8311 not ready");
        _ready = false;
        return false;
    }
    _ready = true;
    DEBUG_PRINTLN("[Mic] ready");
    return true;
}

void Mic::startTest(uint16_t ms) {
    if (!_ready) {
        DEBUG_PRINTLN("[Mic] startTest skipped (not ready)");
        return;
    }
    if (ms < 200) ms = 200;
    _level = 0;
    _testUntil = millis() + ms;
    DEBUG_PRINTLN("[Mic] startTest");
}

void Mic::poll() {
    if (!_ready || !testActive()) {
        _level = 0;
        return;
    }

    int16_t buf[128 * 2];
    size_t frames = Es8311::readStereo(buf, 128, 15);
    if (frames == 0) {
        if (_level > 4) _level = (uint8_t)(_level - 4);
        else _level = 0;
        return;
    }

    uint32_t peak = 0;
    for (size_t i = 0; i < frames; i++) {
        int16_t l = buf[i * 2];
        int16_t r = buf[i * 2 + 1];
        uint32_t a = (uint32_t)(l < 0 ? -l : l);
        uint32_t b = (uint32_t)(r < 0 ? -r : r);
        if (b > a) a = b;
        if (a > peak) peak = a;
    }

    uint8_t pct = (uint8_t)((peak * 100) / 32768);
    if (pct > 100) pct = 100;
    if (pct > _level) _level = pct;
    else _level = (uint8_t)((_level * 3 + pct) / 4);
}

#else

bool Mic::begin() {
    DEBUG_PRINTLN("[Mic] HAS_MIC=0");
    _ready = false;
    return false;
}

void Mic::startTest(uint16_t) {}
void Mic::poll() {}

#endif
