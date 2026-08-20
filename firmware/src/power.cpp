#include "power.h"
#include "display.h"
#include "boards/board.h"
#include <Wire.h>
#include <esp_sleep.h>

void Power::restart() {
    DEBUG_PRINTLN("[Power] restart");
    delay(150);
    ESP.restart();
}

void Power::off() {
    DEBUG_PRINTLN("[Power] off");
    Display::fillScreen(0);
    Display::setBrightness(0);
#if HAS_PMU
    Wire.beginTransmission(PMU_I2C_ADDR);
    Wire.write(0x10);
    if (Wire.endTransmission(false) == 0 &&
        Wire.requestFrom((uint8_t)PMU_I2C_ADDR, (uint8_t)1) == 1) {
        uint8_t v = (uint8_t)Wire.read();
        Wire.beginTransmission(PMU_I2C_ADDR);
        Wire.write(0x10);
        Wire.write(v | 0x01);
        Wire.endTransmission();
    }
#endif
    delay(200);
    esp_deep_sleep_start();
}
