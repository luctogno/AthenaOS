#ifndef ATHENAOS_IMU_H
#define ATHENAOS_IMU_H

#include <Arduino.h>
#include "boards/board.h"

class Imu {
public:
    static bool begin();
    static bool readAccelG(float &ax, float &ay, float &az);
    static bool isReady() { return _ready; }
    static uint8_t address() { return _addr; }

private:
    static bool _ready;
    static uint8_t _addr;
    static bool probe(uint8_t addr);
    static bool writeReg(uint8_t reg, uint8_t val);
    static bool readRegs(uint8_t reg, uint8_t *buf, uint8_t len);
};

#endif
