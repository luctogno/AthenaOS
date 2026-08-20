#include "tamafi_input.h"
#include "touch.h"
#include "config.h"
#include <Wire.h>

static bool bootLast = HIGH;
static bool pwrDown = false;
static unsigned long pwrDownAt = 0;
static uint8_t tcaAddr = 0;
static bool tcaOk = false;

static bool touchWasDown = false;
static uint16_t touchStartX = 0, touchStartY = 0;
static uint16_t touchLastX = 0, touchLastY = 0;
static unsigned long touchStartMs = 0;

static bool tcaProbe(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

static uint8_t tcaRead(uint8_t reg) {
    Wire.beginTransmission(tcaAddr);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return 0;
    if (Wire.requestFrom(tcaAddr, (uint8_t)1) != 1) return 0;
    return Wire.read();
}

static bool tcaWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(tcaAddr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

static bool pwrPressedNow() {
    if (!tcaOk) return false;
    uint8_t in = tcaRead(0x00);
    return (in & (1 << TCA9554_EXIO_PWR)) != 0;
}

bool TamafiInput::begin() {
    pinMode(BTN_BOOT_PIN, INPUT_PULLUP);
    bootLast = digitalRead(BTN_BOOT_PIN);

    if (tcaProbe(TCA9554_ADDR)) tcaAddr = TCA9554_ADDR;
    else if (tcaProbe(0x21)) tcaAddr = 0x21;
    else tcaAddr = 0;

    if (tcaAddr) {
        uint8_t cfg = tcaRead(0x03);
        tcaWrite(0x03, cfg | (1 << TCA9554_EXIO_PWR));
        tcaOk = true;
        DEBUG_PRINTF("[Input] TCA9554 @ 0x%02X cfg=0x%02X PWR=%d\n",
                     tcaAddr, cfg, pwrPressedNow());
    } else {
        DEBUG_PRINTLN("[Input] TCA9554 not found — PWR button disabled");
    }

    DEBUG_PRINTF("[Input] BOOT gpio=%d level=%d (LOW=pressed)\n",
                 BTN_BOOT_PIN, digitalRead(BTN_BOOT_PIN));
    return true;
}

TamafiInputEvent TamafiInput::poll() {
    TamafiInputEvent e;

    bool boot = digitalRead(BTN_BOOT_PIN);
    if (bootLast == HIGH && boot == LOW) e.ok = true;
    bootLast = boot;

    bool pwr = pwrPressedNow();
    unsigned long now = millis();
    if (pwr && !pwrDown) {
        pwrDown = true;
        pwrDownAt = now;
    } else if (!pwr && pwrDown) {
        unsigned long held = now - pwrDownAt;
        pwrDown = false;
        if (held >= 40 && held < 800) e.down = true;
    } else if (pwr && pwrDown && (now - pwrDownAt) > 4000) {
        // approaching AXP 6s power-off — ignore this press
        pwrDown = false;
    }

    uint16_t tx = 0, ty = 0;
    bool down = Touch::read(&tx, &ty);
    if (down) {
        if (!touchWasDown) {
            touchWasDown = true;
            touchStartX = tx;
            touchStartY = ty;
            touchStartMs = now;
        }
        touchLastX = tx;
        touchLastY = ty;
    } else if (touchWasDown) {
        touchWasDown = false;
        int16_t dx = (int16_t)touchLastX - (int16_t)touchStartX;
        int16_t dy = (int16_t)touchLastY - (int16_t)touchStartY;
        unsigned long dur = now - touchStartMs;
        bool hudBtn = (int16_t)touchStartY >= HUD_BTN_Y;

        // Bottom REST/PLAY are flush to the bezel: lift/roll would look like a swipe
        // or land in the stats rect if we used the release point.
        if (!hudBtn && abs(dy) > 40 && abs(dy) > abs(dx)) {
            if (dy < 0) e.up = true;
            else e.down = true;
        } else if (dur < 800 && (hudBtn || (abs(dx) < 28 && abs(dy) < 28))) {
            e.tap = true;
            e.tapX = (int16_t)touchStartX;
            e.tapY = (int16_t)touchStartY;
        }
    }

    return e;
}
