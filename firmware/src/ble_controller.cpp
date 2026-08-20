#include "ble_controller.h"

bool BleController::_ready = false;

#if HAS_BLE

bool BleController::begin() {
    DEBUG_PRINTF("[BLE] stub radio base name=%s classic=%d (no NimBLE yet)\n",
                 BLE_DEVICE_NAME, HAS_CLASSIC_BT);
    _ready = true;
    return true;
}

void BleController::end() {
    _ready = false;
}

#else

bool BleController::begin() {
    DEBUG_PRINTLN("[BLE] HAS_BLE=0");
    _ready = false;
    return false;
}

void BleController::end() {
    _ready = false;
}

#endif
