#include "input.h"
#include "touch.h"
#include "boards/board.h"
#include <Wire.h>

static int16_t iabs16(int16_t v) {
    return v < 0 ? (int16_t)-v : v;
}

static bool btn1Last = HIGH;
static bool btn2Last = HIGH;
static bool btn3Last = HIGH;

static bool pwrDown = false;
static unsigned long pwrDownAt = 0;
static uint8_t tcaAddr = 0;
static bool tcaOk = false;

static bool touchWasDown = false;
static uint16_t touchStartX = 0, touchStartY = 0;
static uint16_t touchLastX = 0, touchLastY = 0;
static unsigned long touchStartMs = 0;

#if HAS_PWR_EXPANDER
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
#endif

static bool edgePress(int pin, bool &last) {
    if (pin < 0) return false;
    bool now = digitalRead(pin);
    bool fired = (last == HIGH && now == LOW);
    last = now;
    return fired;
}

bool Input::begin() {
#if BTN1_PIN >= 0
    pinMode(BTN1_PIN, INPUT_PULLUP);
    btn1Last = digitalRead(BTN1_PIN);
#endif
#if BTN2_PIN >= 0
    pinMode(BTN2_PIN, INPUT_PULLUP);
    btn2Last = digitalRead(BTN2_PIN);
#endif
#if BTN3_PIN >= 0
    pinMode(BTN3_PIN, INPUT_PULLUP);
    btn3Last = digitalRead(BTN3_PIN);
#endif

#if HAS_PWR_EXPANDER
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
#endif

#if BTN1_PIN >= 0
    DEBUG_PRINTF("[Input] BTN1 gpio=%d level=%d (LOW=pressed)\n",
                 BTN1_PIN, digitalRead(BTN1_PIN));
#endif
    return true;
}

InputEvent Input::poll() {
    InputEvent e;
    unsigned long now = millis();

    e.btn1 = edgePress(BTN1_PIN, btn1Last);
    e.btn2 = edgePress(BTN2_PIN, btn2Last);
    e.btn3 = edgePress(BTN3_PIN, btn3Last);

#if HAS_PWR_EXPANDER
    bool pwr = pwrPressedNow();
    if (pwr && !pwrDown) {
        pwrDown = true;
        pwrDownAt = now;
    } else if (!pwr && pwrDown) {
        unsigned long held = now - pwrDownAt;
        pwrDown = false;
        if (held >= 40 && held < 800) e.btn2 = true;
    } else if (pwr && pwrDown && (now - pwrDownAt) > 4000) {
        pwrDown = false;
    }
#endif

#if HAS_TOUCH
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
        if (dur < 800 && iabs16(dx) < 28 && iabs16(dy) < 28) {
            e.tap = true;
            e.tapX = (int16_t)touchStartX;
            e.tapY = (int16_t)touchStartY;
        }
    }
#endif

    return e;
}
