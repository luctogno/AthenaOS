#include "clock.h"
#include "settings.h"
#include "boards/board.h"

#include <Preferences.h>
#include <Wire.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>

static unsigned long lastSaveMs = 0;
static time_t lastEpochSaved = 0;

static uint8_t dec2bcd(int v) {
    if (v < 0) v = 0;
    return (uint8_t)(((v / 10) << 4) | (v % 10));
}

static int bcd2dec(uint8_t v) {
    return (int)((v & 0x0F) + ((v >> 4) & 0x0F) * 10);
}

static void applyPosixTz() {
    setenv("TZ", Settings::tzPosix(), 1);
    tzset();
}

static void setEpoch(time_t epoch) {
    if (epoch <= 0) return;
    struct timeval tv = {epoch, 0};
    settimeofday(&tv, nullptr);
}

static void nvsSave(time_t epoch) {
    if (epoch < 1704067200) return;
    Preferences p;
    if (!p.begin("clock", false)) return;
    p.putULong("epoch", (uint32_t)epoch);
    p.end();
}

static bool nvsLoad(time_t &epoch) {
    Preferences p;
    if (!p.begin("clock", true)) return false;
    uint32_t e = p.getULong("epoch", 0);
    p.end();
    if (e < 1704067200UL) return false;
    epoch = (time_t)e;
    return true;
}

#if HAS_RTC
static bool rtcRead(struct tm &out) {
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x04);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)RTC_I2C_ADDR, (uint8_t)7) != 7) return false;
    uint8_t sec = Wire.read();
    uint8_t min = Wire.read();
    uint8_t hour = Wire.read();
    uint8_t day = Wire.read();
    uint8_t wday = Wire.read();
    uint8_t month = Wire.read();
    uint8_t year = Wire.read();
    if (sec & 0x80) return false;
    memset(&out, 0, sizeof(out));
    out.tm_sec = bcd2dec(sec & 0x7F);
    out.tm_min = bcd2dec(min & 0x7F);
    out.tm_hour = bcd2dec(hour & 0x3F);
    out.tm_mday = bcd2dec(day & 0x3F);
    out.tm_wday = bcd2dec(wday & 0x07);
    out.tm_mon = bcd2dec(month & 0x1F) - 1;
    out.tm_year = bcd2dec(year) + 100;
    out.tm_isdst = -1;
    if (out.tm_sec > 59 || out.tm_min > 59 || out.tm_hour > 23) return false;
    if (out.tm_mon < 0 || out.tm_mon > 11 || out.tm_mday < 1 || out.tm_mday > 31) return false;
    if (out.tm_year < 124) return false;
    return true;
}

static bool rtcWrite(const struct tm &t) {
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x00);
    Wire.write(0x01);
    if (Wire.endTransmission() != 0) return false;

    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x04);
    Wire.write(dec2bcd(t.tm_sec));
    Wire.write(dec2bcd(t.tm_min));
    Wire.write(dec2bcd(t.tm_hour));
    Wire.write(dec2bcd(t.tm_mday));
    Wire.write(dec2bcd(t.tm_wday));
    Wire.write(dec2bcd(t.tm_mon + 1));
    Wire.write(dec2bcd(t.tm_year % 100));
    if (Wire.endTransmission() != 0) return false;
    DEBUG_PRINTF("[Clock] RTC saved %04d-%02d-%02d %02d:%02d\n",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
    return true;
}

static bool rtcLoadToSystem() {
    struct tm t;
    if (!rtcRead(t)) return false;
    time_t epoch = mktime(&t);
    if (epoch < 1704067200) return false;
    setEpoch(epoch);
    DEBUG_PRINTF("[Clock] RTC load %02d:%02d\n", t.tm_hour, t.tm_min);
    return true;
}
#endif

static bool localNow(struct tm &t) {
    return getLocalTime(&t, 0) && t.tm_year + 1900 >= 2024;
}

static void persist(const struct tm &t) {
    nvsSave(time(nullptr));
#if HAS_RTC
    rtcWrite(t);
#endif
}

void Clock::applyTz() {
    applyPosixTz();
    lastSaveMs = 0;
}

void Clock::begin() {
    applyPosixTz();
#if HAS_RTC
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x00);
    Wire.write(0x01);
    Wire.endTransmission();
    if (rtcLoadToSystem()) {
        DEBUG_PRINTLN("[Clock] time from RTC");
        return;
    }
#endif
    time_t epoch = 0;
    if (nvsLoad(epoch)) {
        setEpoch(epoch);
        DEBUG_PRINTLN("[Clock] time from NVS");
        return;
    }
    DEBUG_PRINTLN("[Clock] no saved time, wait NTP");
}

void Clock::poll() {
    struct tm t;
    if (!localNow(t)) return;
    time_t now = time(nullptr);
    bool due = lastSaveMs == 0 || millis() - lastSaveMs >= 30000UL;
    bool jumped = lastEpochSaved != 0 &&
                  (now > lastEpochSaved + 90 || lastEpochSaved > now + 90);
    if (!due && !jumped) return;
    lastSaveMs = millis();
    lastEpochSaved = now;
    persist(t);
}

void Clock::format(char *buf, size_t len) {
    struct tm t;
    if (localNow(t)) {
        snprintf(buf, len, "%02d:%02d", t.tm_hour, t.tm_min);
        return;
    }
#if HAS_RTC
    if (rtcRead(t)) {
        snprintf(buf, len, "%02d:%02d", t.tm_hour, t.tm_min);
        return;
    }
#endif
    snprintf(buf, len, "--:--");
}

bool Clock::hasTime() {
    struct tm t;
    return localNow(t);
}
