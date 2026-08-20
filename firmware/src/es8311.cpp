#include "es8311.h"

#if HAS_AUDIO || HAS_MIC

#include <Wire.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>

bool Es8311::_ready = false;

static i2s_chan_handle_t txHandle = nullptr;
static i2s_chan_handle_t rxHandle = nullptr;
static bool paOn = false;
static bool i2sStarted = false;

bool Es8311::writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission((uint8_t)AUDIO_I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool Es8311::readReg(uint8_t reg, uint8_t &val) {
    Wire.beginTransmission((uint8_t)AUDIO_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)AUDIO_I2C_ADDR, (uint8_t)1) != 1) return false;
    val = (uint8_t)Wire.read();
    return true;
}

bool Es8311::probe() {
    uint8_t id1 = 0, id2 = 0;
    if (!readReg(0xFD, id1) || !readReg(0xFE, id2)) return false;
    return id1 == 0x83 && id2 == 0x11;
}

void Es8311::enableMicLdo() {
#if HAS_PMU
    Wire.beginTransmission((uint8_t)PMU_I2C_ADDR);
    Wire.write(0x92);
    Wire.write((uint8_t)((3300 - 500) / 100));
    Wire.endTransmission();

    Wire.beginTransmission((uint8_t)PMU_I2C_ADDR);
    Wire.write(0x90);
    if (Wire.endTransmission(false) != 0) return;
    if (Wire.requestFrom((uint8_t)PMU_I2C_ADDR, (uint8_t)1) != 1) return;
    uint8_t v = (uint8_t)Wire.read();
    if (v & 0x01) return;
    Wire.beginTransmission((uint8_t)PMU_I2C_ADDR);
    Wire.write(0x90);
    Wire.write(v | 0x01);
    Wire.endTransmission();
    delay(20);
#endif
}

bool Es8311::initI2s() {
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chanCfg.dma_desc_num = 4;
    chanCfg.dma_frame_num = 256;

#if HAS_AUDIO && HAS_MIC
    if (i2s_new_channel(&chanCfg, &txHandle, &rxHandle) != ESP_OK) return false;
#elif HAS_AUDIO
    if (i2s_new_channel(&chanCfg, &txHandle, nullptr) != ESP_OK) return false;
#else
    if (i2s_new_channel(&chanCfg, nullptr, &rxHandle) != ESP_OK) return false;
#endif

    i2s_std_config_t stdCfg = {};
    stdCfg.clk_cfg.sample_rate_hz = (uint32_t)I2S_SAMPLE_RATE;
    stdCfg.clk_cfg.clk_src = I2S_CLK_SRC_DEFAULT;
#ifdef I2S_HW_VERSION_2
    stdCfg.clk_cfg.ext_clk_freq_hz = 0;
#endif
    stdCfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
    stdCfg.slot_cfg.data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;
    stdCfg.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO;
    stdCfg.slot_cfg.slot_mode = I2S_SLOT_MODE_STEREO;
    stdCfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
    stdCfg.slot_cfg.ws_width = I2S_DATA_BIT_WIDTH_16BIT;
    stdCfg.slot_cfg.ws_pol = false;
    stdCfg.slot_cfg.bit_shift = true;
#ifdef I2S_HW_VERSION_2
    stdCfg.slot_cfg.left_align = true;
    stdCfg.slot_cfg.big_endian = false;
    stdCfg.slot_cfg.bit_order_lsb = false;
#endif
    stdCfg.gpio_cfg.mclk = (gpio_num_t)I2S_MCLK_PIN;
    stdCfg.gpio_cfg.bclk = (gpio_num_t)I2S_BCLK_PIN;
    stdCfg.gpio_cfg.ws = (gpio_num_t)I2S_LRC_PIN;
#if HAS_AUDIO
    stdCfg.gpio_cfg.dout = (gpio_num_t)I2S_DIN_PIN;
#else
    stdCfg.gpio_cfg.dout = GPIO_NUM_NC;
#endif
#if HAS_MIC
    stdCfg.gpio_cfg.din = (gpio_num_t)I2S_DOUT_PIN;
#else
    stdCfg.gpio_cfg.din = GPIO_NUM_NC;
#endif

    if (txHandle && i2s_channel_init_std_mode(txHandle, &stdCfg) != ESP_OK) return false;
    if (rxHandle && i2s_channel_init_std_mode(rxHandle, &stdCfg) != ESP_OK) return false;
    if (txHandle && i2s_channel_enable(txHandle) != ESP_OK) return false;
    if (rxHandle && i2s_channel_enable(rxHandle) != ESP_OK) return false;
    return true;
}

bool Es8311::initCodec() {
    if (!writeReg(0x00, 0x1F)) return false;
    delay(5);
    if (!writeReg(0x00, 0x00)) return false;

    // 16 kHz, MCLK = 256 * Fs = 4.096 MHz (ESPHome / ESP-ADF coeff table)
    if (!writeReg(0x01, 0x3F)) return false;
    if (!writeReg(0x02, 0x08)) return false;
    if (!writeReg(0x03, 0x10)) return false;
    if (!writeReg(0x04, 0x20)) return false;
    if (!writeReg(0x05, 0x00)) return false;
    if (!writeReg(0x06, 0x03)) return false;
    if (!writeReg(0x07, 0x00)) return false;
    if (!writeReg(0x08, 0xFF)) return false;

    if (!writeReg(0x09, 0x0C)) return false;
    if (!writeReg(0x0A, 0x0C)) return false;

    if (!writeReg(0x0D, 0x01)) return false;
    if (!writeReg(0x0E, 0x02)) return false;
    if (!writeReg(0x12, 0x00)) return false;
    if (!writeReg(0x13, 0x10)) return false;
    if (!writeReg(0x14, 0x1A)) return false;
    if (!writeReg(0x16, 0x00)) return false;
    if (!writeReg(0x17, 0xC8)) return false;
    if (!writeReg(0x1C, 0x6A)) return false;
    if (!writeReg(0x37, 0x08)) return false;
    if (!writeReg(0x00, 0x80)) return false;
    if (!writeReg(0x32, 0xBF)) return false;
    setMute(true);
    return true;
}

bool Es8311::begin() {
    if (_ready) return true;

    enableMicLdo();
#if I2S_PA_PIN >= 0
    pinMode(I2S_PA_PIN, OUTPUT);
    digitalWrite(I2S_PA_PIN, LOW);
#endif

    if (!i2sStarted) {
        if (!initI2s()) {
            DEBUG_PRINTLN("[ES8311] I2S init failed");
            return false;
        }
        i2sStarted = true;
        delay(10);
    }

    if (!probe()) {
        DEBUG_PRINTF("[ES8311] not found @ 0x%02X\n", AUDIO_I2C_ADDR);
        return false;
    }
    if (!initCodec()) {
        DEBUG_PRINTLN("[ES8311] codec init failed");
        return false;
    }

    _ready = true;
    DEBUG_PRINTF("[ES8311] ready @ 0x%02X I2S %dHz PA=%d\n",
                 AUDIO_I2C_ADDR, I2S_SAMPLE_RATE, I2S_PA_PIN);
    return true;
}

void Es8311::setVolume(uint8_t percent) {
    if (percent > 100) percent = 100;
    uint8_t dac = (uint8_t)((percent * 255) / 100);
    writeReg(0x32, dac);
}

void Es8311::setPa(bool on) {
#if I2S_PA_PIN >= 0
    if (on == paOn) return;
    paOn = on;
    digitalWrite(I2S_PA_PIN, on ? HIGH : LOW);
#else
    (void)on;
#endif
}

void Es8311::setMute(bool mute) {
    uint8_t v = 0;
    if (!readReg(0x31, v)) return;
    if (mute) v |= 0x60;
    else v &= (uint8_t)~0x60;
    writeReg(0x31, v);
}

bool Es8311::writeStereo(const int16_t *frames, size_t count, uint32_t timeoutMs) {
    if (!_ready || !txHandle || !frames || count == 0) return false;
    size_t written = 0;
    return i2s_channel_write(txHandle, frames, count * 4, &written, timeoutMs) == ESP_OK;
}

size_t Es8311::readStereo(int16_t *frames, size_t count, uint32_t timeoutMs) {
    if (!_ready || !rxHandle || !frames || count == 0) return 0;
    size_t n = 0;
    if (i2s_channel_read(rxHandle, frames, count * 4, &n, timeoutMs) != ESP_OK) return 0;
    return n / 4;
}

#else

bool Es8311::_ready = false;

bool Es8311::begin() { return false; }
bool Es8311::probe() { return false; }
bool Es8311::writeReg(uint8_t, uint8_t) { return false; }
bool Es8311::readReg(uint8_t, uint8_t &) { return false; }
bool Es8311::initI2s() { return false; }
bool Es8311::initCodec() { return false; }
void Es8311::enableMicLdo() {}
void Es8311::setVolume(uint8_t) {}
void Es8311::setPa(bool) {}
void Es8311::setMute(bool) {}
bool Es8311::writeStereo(const int16_t *, size_t, uint32_t) { return false; }
size_t Es8311::readStereo(int16_t *, size_t, uint32_t) { return 0; }

#endif
