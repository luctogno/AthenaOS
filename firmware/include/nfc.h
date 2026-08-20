#ifndef ATHENAOS_NFC_H
#define ATHENAOS_NFC_H

#include <Arduino.h>
#include "boards/board.h"

class Nfc {
public:
    static bool begin();
    static bool isReady() { return _ready; }

private:
    static bool _ready;
};

#endif
