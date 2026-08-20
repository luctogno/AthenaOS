#include "pedometer.h"
#include "imu.h"

uint32_t Pedometer::_total = 0;
bool Pedometer::_armed = false;
float Pedometer::_filt = 1.0f;
unsigned long Pedometer::_lastStepMs = 0;
unsigned long Pedometer::_lastPollMs = 0;

#if HAS_PEDOMETER
#include <math.h>

bool Pedometer::begin() {
    _armed = false;
    _filt = 1.0f;
    _lastStepMs = 0;
    _lastPollMs = 0;
    if (Imu::isReady()) return true;
    return Imu::begin();
}

bool Pedometer::imuReady() {
    return Imu::isReady();
}

void Pedometer::poll() {
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
    _total++;
}

#else

bool Pedometer::begin() {
    DEBUG_PRINTLN("[Pedometer] HAS_PEDOMETER=0");
    return false;
}

bool Pedometer::imuReady() {
    return false;
}

void Pedometer::poll() {}

#endif
