#include "audio.h"
#include "es8311.h"
#include "mic.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool Audio::_ready = false;
uint8_t Audio::_volume = 70;
unsigned long Audio::_testUntil = 0;

void Audio::setVolume(uint8_t percent) {
    if (percent > 100) percent = 100;
    _volume = percent;
    if (_ready) Es8311::setVolume(_volume);
    DEBUG_PRINTF("[Audio] volume=%u\n", (unsigned)_volume);
}

bool Audio::testActive() {
    return _ready && (long)(millis() - _testUntil) < 0;
}

#if HAS_AUDIO

static uint8_t sineIdx = 0;
static bool paLatched = false;
static bool taskStarted = false;

static const int16_t SINE[32] = {
    0, 6393, 12540, 18205, 23170, 27246, 30274, 32138,
    32767, 32138, 30274, 27246, 23170, 18205, 12540, 6393,
    0, -6393, -12540, -18205, -23170, -27246, -30274, -32138,
    -32767, -32138, -30274, -27246, -23170, -18205, -12540, -6393
};

static void fillTone(int16_t *dst, size_t frames) {
    for (size_t i = 0; i < frames; i++) {
        int16_t s = (int16_t)((SINE[sineIdx] * 5) / 8);
        sineIdx = (uint8_t)((sineIdx + 2) & 31);
        dst[i * 2] = s;
        dst[i * 2 + 1] = s;
    }
}

static void codecTask(void *) {
    for (;;) {
        Audio::poll();
        Mic::poll();
        if (!Audio::testActive() && !Mic::testActive()) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

bool Audio::begin() {
    if (!Es8311::begin()) {
        DEBUG_PRINTLN("[Audio] ES8311 not ready");
        _ready = false;
        return false;
    }
    Es8311::setVolume(_volume);
    _ready = true;
    if (!taskStarted) {
        taskStarted = true;
        xTaskCreatePinnedToCore(codecTask, "codec", 4096, nullptr, 5, nullptr, 1);
    }
    DEBUG_PRINTLN("[Audio] ready");
    return true;
}

void Audio::playTest(uint16_t ms) {
    if (!_ready) {
        DEBUG_PRINTLN("[Audio] playTest skipped (not ready)");
        return;
    }
    if (ms < 200) ms = 200;
    sineIdx = 0;
    _testUntil = millis() + ms;
    Es8311::setVolume(_volume);
    Es8311::setMute(false);
    Es8311::setPa(true);
    paLatched = true;
    DEBUG_PRINTF("[Audio] playTest %ums vol=%u\n", (unsigned)ms, (unsigned)_volume);
}

void Audio::poll() {
    if (!_ready) return;
    bool on = testActive();
    if (on) {
        int16_t buf[128 * 2];
        fillTone(buf, 128);
        Es8311::writeStereo(buf, 128, 20);
        return;
    }
    if (paLatched) {
        int16_t z[64 * 2] = {};
        Es8311::writeStereo(z, 64, 0);
        Es8311::setMute(true);
        Es8311::setPa(false);
        paLatched = false;
    }
}

#else

bool Audio::begin() {
    DEBUG_PRINTLN("[Audio] HAS_AUDIO=0");
    _ready = false;
    return false;
}

void Audio::playTest(uint16_t) {}
void Audio::poll() {}

#endif
