#include <Arduino.h>
#include <Wire.h>

#include "boards/board.h"
#include "display.h"
#include "touch.h"
#include "input.h"
#include "imu.h"
#include "pedometer.h"
#include "audio.h"
#include "mic.h"
#include "nfc.h"
#include "ble_controller.h"
#include "splash.h"
#include "app_manager.h"
#include "settings.h"
#include "status_bar.h"
#include "web_console.h"
#include "log_buffer.h"
#include "clock.h"
#include <string.h>

extern void registerLauncherApp();
extern void registerTamafiApp();
extern void registerXiaozhiApp();
extern void registerSettingsApp();

static bool splashDone = false;
static unsigned long homeHoldStart = 0;

static bool pollHomeHold() {
    App *cur = g_appManager.getCurrentApp();
    if (!cur || HOME_HOLD_MS == 0 || BTN1_PIN < 0) return false;
    AppManifest m = cur->getManifest();
    if (m.id && strcmp(m.id, "launcher") == 0) {
        homeHoldStart = 0;
        return false;
    }

    if (digitalRead(BTN1_PIN) == LOW) {
        if (homeHoldStart == 0) homeHoldStart = millis();
        else if (millis() - homeHoldStart >= HOME_HOLD_MS) {
            homeHoldStart = 0;
            g_appManager.switchToAppById("launcher");
            return true;
        }
    } else {
        homeHoldStart = 0;
    }
    return false;
}

static void probeDevices() {
    DEBUG_PRINTLN("=== AthenaOS probe ===");
    DEBUG_PRINTF("board=%s v2=%d %dx%d\n", BOARD_NAME, WAVESHARE_AMOLED_V2,
                 SCREEN_WIDTH, SCREEN_HEIGHT);

    bool displayOk = Display::begin();
    bool touchOk = Touch::begin();
    bool inputOk = Input::begin();
    bool imuOk = Imu::begin();
    bool pedOk = Pedometer::begin();
    bool audioOk = Audio::begin();
    bool micOk = Mic::begin();
    bool nfcOk = Nfc::begin();
    bool bleOk = BleController::begin();

    DEBUG_PRINTF("display=%d touch=%d input=%d imu=%d pedometer=%d\n",
                 displayOk, touchOk, inputOk, imuOk, pedOk);
    DEBUG_PRINTF("audio=%s mic=%s nfc=%d ble=%s pmu_cfg=%d rtc_cfg=%d sd_cfg=%d\n",
                 audioOk ? "stub" : "0",
                 micOk ? "stub" : "0",
                 nfcOk,
                 bleOk ? "stub" : "0",
                 HAS_PMU, HAS_RTC, HAS_SD);
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);
    LogBuffer::begin();
    DEBUG_PRINTLN("\n=== AthenaOS Booting ===");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);

    probeDevices();
    Settings::begin();
    Clock::begin();

    registerLauncherApp();
    registerTamafiApp();
    registerXiaozhiApp();
    registerSettingsApp();

    Splash::begin();
    DEBUG_PRINTLN("AthenaOS ready");
}

void loop() {
    Settings::poll();
    Clock::poll();
    WebConsole::poll();

    if (!splashDone) {
        if (Splash::update()) {
            splashDone = true;
            g_appManager.switchToAppById("launcher");
        }
        return;
    }

    if (pollHomeHold()) return;

    App *app = g_appManager.getCurrentApp();
    if (!app) return;

    if (!app->consumesInput()) {
        InputEvent e = Input::poll();
        if (e.tap) app->onTouchDown((uint16_t)e.tapX, (uint16_t)e.tapY);
        if (e.btn1) app->onButton(1);
        if (e.btn2) app->onButton(2);
        if (e.btn3) app->onButton(3);
    }

    app->update();
    app->draw();
    StatusBar::tick();
}
