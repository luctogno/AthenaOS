#include "touch.h"

bool Touch::_ready = false;
uint8_t Touch::_addr = TOUCH_I2C_ADDR;

#if HAS_TOUCH
#include <Wire.h>

static bool i2cProbe(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

bool Touch::begin() {
    if (TOUCH_PIN_INT >= 0) {
        pinMode(TOUCH_PIN_INT, INPUT_PULLUP);
    }

    bool found_v1 = i2cProbe(0x38);
    bool found_v2 = i2cProbe(0x15);
    if (i2cProbe(TOUCH_I2C_ADDR)) {
        _addr = TOUCH_I2C_ADDR;
    } else if (found_v1) {
        _addr = 0x38;
    } else if (found_v2) {
        _addr = 0x15;
    }

    _ready = i2cProbe(_addr);
    DEBUG_PRINTF("[Touch] addr=0x%02X ready=%d (0x38=%d 0x15=%d)\n",
                 _addr, _ready, found_v1, found_v2);
    return _ready;
}

bool Touch::read(uint16_t *x, uint16_t *y) {
    if (!_ready) return false;

    Wire.beginTransmission(_addr);
    Wire.write(0x02);
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(_addr, (uint8_t)5) < 5) return false;

    uint8_t raw[5];
    for (int i = 0; i < 5; i++) raw[i] = Wire.read();
    if ((raw[0] & 0x0F) == 0) return false;

    uint16_t px = ((raw[1] & 0x0F) << 8) | raw[2];
    uint16_t py = ((raw[3] & 0x0F) << 8) | raw[4];
    if (px >= TFT_WIDTH) px = TFT_WIDTH - 1;
    if (py >= TFT_HEIGHT) py = TFT_HEIGHT - 1;
    *x = px;
    *y = py;
    return true;
}

#else

bool Touch::begin() {
    DEBUG_PRINTLN("[Touch] HAS_TOUCH=0");
    _ready = false;
    return false;
}

bool Touch::read(uint16_t *, uint16_t *) {
    return false;
}

#endif
