#include "status_bar.h"
#include "display.h"
#include "settings.h"
#include "clock.h"
#include "boards/board.h"

#include <WiFi.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>

static char lastTime[8] = "";
static bool barVisible = false;
static int lastWifiBars = -2;
static int lastBatt = -200;

static int batteryPercent() {
#if HAS_PMU
    Wire.beginTransmission(PMU_I2C_ADDR);
    Wire.write(0xA4);
    if (Wire.endTransmission(false) != 0) return -1;
    if (Wire.requestFrom((uint8_t)PMU_I2C_ADDR, (uint8_t)1) != 1) return -1;
    int pct = Wire.read();
    if (pct < 0) return -1;
    if (pct > 100) pct = 100;
    return pct;
#else
    return -1;
#endif
}

static int wifiBars() {
    if (!Settings::wifiEnabled() || WiFi.getMode() == WIFI_OFF) return -1;
    if (WiFi.status() != WL_CONNECTED) return 0;
    int rssi = WiFi.RSSI();
    if (rssi > -60) return 3;
    if (rssi > -75) return 2;
    return 1;
}

static void drawWifiIcon(int16_t x, int16_t y) {
    int bars = wifiBars();
    uint16_t on = COLOR_SECOND;
    uint16_t off = COLOR_MUTED;
    for (int i = 0; i < 3; i++) {
        int16_t h = 6 + i * 6;
        uint16_t c = off;
        if (bars > 0 && bars > i) c = on;
        Display::fillRect(x + i * 7, y + 18 - h, 5, h, c);
    }
    if (bars < 0) {
        Display::drawLine(x, y + 2, x + 20, y + 16, COLOR_ERROR);
    }
}

static void drawBatteryIcon(int16_t x, int16_t y) {
    int pct = batteryPercent();
    Display::drawRect(x, y, 22, 12, COLOR_FG);
    Display::fillRect(x + 22, y + 3, 3, 6, COLOR_FG);
    int fill = 0;
    if (pct >= 0) fill = (18 * pct) / 100;
    uint16_t c = COLOR_SECOND;
    if (pct >= 0 && pct <= 15) c = COLOR_ERROR;
    else if (pct < 0) c = COLOR_MUTED;
    if (fill < 1 && pct > 0) fill = 1;
    if (pct < 0) fill = 8;
    if (fill > 0) Display::fillRect(x + 2, y + 2, fill, 8, c);
}

static void drawTime(const char *timeBuf) {
    Display::fillRect(140, 0, 100, StatusBar::HEIGHT, COLOR_PANEL);
    Display::drawText(148, 16, timeBuf, COLOR_FG, FONT_UI);
}

static void redrawWifi() {
    lastWifiBars = wifiBars();
    Display::fillRect(SCREEN_WIDTH - 72, 12, 34, 28, COLOR_PANEL);
    drawWifiIcon(SCREEN_WIDTH - 70, 16);
}

static void redrawBattery() {
    lastBatt = batteryPercent();
    Display::fillRect(SCREEN_WIDTH - 38, 16, 38, 22, COLOR_PANEL);
    drawBatteryIcon(SCREEN_WIDTH - 36, 20);
}

void StatusBar::draw(const char *title) {
    barVisible = true;

    Display::fillRect(0, 0, SCREEN_WIDTH, HEIGHT, COLOR_PANEL);
    Display::drawText(12, 16, title ? title : "AthenaOS", COLOR_MAIN, FONT_UI);

    Clock::format(lastTime, sizeof(lastTime));
    Display::drawText(148, 16, lastTime, COLOR_FG, FONT_UI);

    lastWifiBars = wifiBars();
    lastBatt = batteryPercent();
    drawWifiIcon(SCREEN_WIDTH - 70, 16);
    drawBatteryIcon(SCREEN_WIDTH - 36, 20);
}

void StatusBar::tick() {
    if (!barVisible) return;

    char timeBuf[8];
    Clock::format(timeBuf, sizeof(timeBuf));
    if (strcmp(timeBuf, lastTime) != 0) {
        strncpy(lastTime, timeBuf, sizeof(lastTime) - 1);
        lastTime[sizeof(lastTime) - 1] = 0;
        drawTime(lastTime);
    }

    if (wifiBars() != lastWifiBars) redrawWifi();
    if (batteryPercent() != lastBatt) redrawBattery();
}

void StatusBar::hide() {
    barVisible = false;
    lastTime[0] = 0;
    lastWifiBars = -2;
    lastBatt = -200;
}
