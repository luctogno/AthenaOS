#ifndef ATHENAOS_BLE_CONTROLLER_H
#define ATHENAOS_BLE_CONTROLLER_H

#include <Arduino.h>
#include "boards/board.h"

class BleController {
public:
    static bool begin();
    static void end();
    static bool isReady() { return _ready; }

private:
    static bool _ready;
};

#endif
