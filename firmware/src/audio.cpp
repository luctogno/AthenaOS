#include "audio.h"

bool Audio::_ready = false;
uint8_t Audio::_volume = 70;

void Audio::setVolume(uint8_t percent) {
    if (percent > 100) percent = 100;
    _volume = percent;
    DEBUG_PRINTF("[Audio] volume=%u\n", (unsigned)_volume);
}

#if HAS_AUDIO

bool Audio::begin() {
    DEBUG_PRINTF("[Audio] stub ES8311 @ 0x%02X I2S BCLK=%d LRC=%d DIN=%d MCLK=%d (no playback yet)\n",
                 AUDIO_I2C_ADDR, I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DIN_PIN, I2S_MCLK_PIN);
    _ready = true;
    return true;
}

#else

bool Audio::begin() {
    DEBUG_PRINTLN("[Audio] HAS_AUDIO=0");
    _ready = false;
    return false;
}

#endif
