#ifndef TAMAFI_STEP_DETECTOR_H
#define TAMAFI_STEP_DETECTOR_H

#include <Arduino.h>

class StepDetector {
public:
    static bool begin();
    static void poll();
    static uint32_t total() { return _total; }
    static uint16_t remainder() { return _remainder; }
    static bool imuReady();
    static int addSteps(int n);
    static void setTotal(uint32_t n) { _total = n; }
    static void setRemainder(uint16_t n) { _remainder = n; }
    static uint8_t multiplier() { return _multiplier; }
    static void setMultiplier(uint8_t m);
    static void cycleMultiplier();
    static uint16_t stepsPerFeed();
    static int stepsUntilFeed();

private:
    static uint32_t _total;
    static uint16_t _remainder;
    static uint8_t _multiplier;
    static bool _armed;
    static float _filt;
    static unsigned long _lastStepMs;
    static unsigned long _lastPollMs;

    static int applySteps(int n);
};

#endif
