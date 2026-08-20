#include "StepDetector.h"
#include "imu.h"
#include "config.h"
#include <math.h>

int feedQty(int qty);

static const uint8_t STEP_MULT_OPTIONS[] = {1, 2, 5, 10, 25, 100, 250};
static const int STEP_MULT_COUNT = 7;

uint32_t StepDetector::_total = 0;
uint16_t StepDetector::_remainder = 0;
uint8_t StepDetector::_multiplier = 1;
bool StepDetector::_armed = false;
float StepDetector::_filt = 1.0f;
unsigned long StepDetector::_lastStepMs = 0;
unsigned long StepDetector::_lastPollMs = 0;

bool StepDetector::begin() {
    _armed = false;
    _filt = 1.0f;
    _lastStepMs = 0;
    _lastPollMs = 0;
    if (Imu::isReady()) return true;
    return Imu::begin();
}

bool StepDetector::imuReady() {
    return Imu::isReady();
}

void StepDetector::setMultiplier(uint8_t m) {
    _multiplier = 1;
    for (int i = 0; i < STEP_MULT_COUNT; i++) {
        if (STEP_MULT_OPTIONS[i] == m) {
            _multiplier = m;
            break;
        }
    }
    uint16_t need = stepsPerFeed();
    if (need > 0 && _remainder >= need) _remainder = need - 1;
}

void StepDetector::cycleMultiplier() {
    int i = 0;
    for (; i < STEP_MULT_COUNT; i++) {
        if (STEP_MULT_OPTIONS[i] == _multiplier) break;
    }
    if (i >= STEP_MULT_COUNT) i = 0;
    else i = (i + 1) % STEP_MULT_COUNT;
    setMultiplier(STEP_MULT_OPTIONS[i]);
}

uint16_t StepDetector::stepsPerFeed() {
    uint32_t n = (uint32_t)STEPS_PER_FEED_BASE * _multiplier;
    if (n < 1) n = 1;
    if (n > 65535) n = 65535;
    return (uint16_t)n;
}

int StepDetector::stepsUntilFeed() {
    uint16_t need = stepsPerFeed();
    if (_remainder >= need) return 0;
    return (int)(need - _remainder);
}

int StepDetector::applySteps(int n) {
    if (n <= 0) return 0;
    _total += (uint32_t)n;
    uint32_t need = stepsPerFeed();
    uint32_t acc = (uint32_t)_remainder + (uint32_t)n;
    int batches = (int)(acc / need);
    _remainder = (uint16_t)(acc % need);
    if (batches <= 0) return 0;
    return feedQty(batches * HUNGER_PER_FEED);
}

int StepDetector::addSteps(int n) {
    if (n < 0) n = 0;
    return applySteps(n);
}

void StepDetector::poll() {
    if (!Imu::isReady()) return;

    unsigned long now = millis();
    if (now - _lastPollMs < 20) return;
    _lastPollMs = now;

    float ax, ay, az;
    if (!Imu::readAccelG(ax, ay, az)) return;

    float mag = sqrtf(ax * ax + ay * ay + az * az);
    _filt = _filt * 0.80f + mag * 0.20f;

    if (!_armed) {
        if (_filt > STEP_PEAK_G) _armed = true;
        return;
    }

    if (_filt > STEP_VALLEY_G) return;
    _armed = false;

    if (now - _lastStepMs < STEP_MIN_INTERVAL_MS) return;
    _lastStepMs = now;
    applySteps(1);
}
