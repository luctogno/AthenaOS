#include "imu.h"

bool Imu::_ready = false;
uint8_t Imu::_addr = IMU_I2C_ADDR;

#if HAS_IMU
#include <Wire.h>

#define QMI8658_WHO_AM_I    0x00
#define QMI8658_WHO_AM_I_V  0x05
#define QMI8658_CTRL1       0x02
#define QMI8658_CTRL2       0x03
#define QMI8658_CTRL7       0x08
#define QMI8658_AX_L        0x35

bool Imu::probe(uint8_t addr) {
    Wire.beginTransmission(addr);
    Wire.write(QMI8658_WHO_AM_I);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(addr, (uint8_t)1) != 1) return false;
    uint8_t id = Wire.read();
    return id == QMI8658_WHO_AM_I_V || id == 0x68;
}

bool Imu::writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool Imu::readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(_addr, len) != len) return false;
    for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
    return true;
}

bool Imu::begin() {
    if (probe(0x6A)) _addr = 0x6A;
    else if (probe(0x6B)) _addr = 0x6B;
    else {
        DEBUG_PRINTLN("[IMU] QMI8658 not found");
        _ready = false;
        return false;
    }

    if (!writeReg(QMI8658_CTRL1, 0x40)) return false;
    if (!writeReg(QMI8658_CTRL2, 0x05)) return false;
    if (!writeReg(QMI8658_CTRL7, 0x01)) return false;
    delay(20);

    _ready = true;
    DEBUG_PRINTF("[IMU] QMI8658 @ 0x%02X ready\n", _addr);
    return true;
}

bool Imu::readAccelG(float &ax, float &ay, float &az) {
    if (!_ready) return false;
    uint8_t raw[6];
    if (!readRegs(QMI8658_AX_L, raw, 6)) return false;
    int16_t x = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t y = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t z = (int16_t)((raw[5] << 8) | raw[4]);
    const float lsbPerG = 16384.0f;
    ax = x / lsbPerG;
    ay = y / lsbPerG;
    az = z / lsbPerG;
    return true;
}

#else

bool Imu::probe(uint8_t) { return false; }
bool Imu::writeReg(uint8_t, uint8_t) { return false; }
bool Imu::readRegs(uint8_t, uint8_t *, uint8_t) { return false; }

bool Imu::begin() {
    DEBUG_PRINTLN("[IMU] HAS_IMU=0");
    _ready = false;
    return false;
}

bool Imu::readAccelG(float &, float &, float &) {
    return false;
}

#endif
