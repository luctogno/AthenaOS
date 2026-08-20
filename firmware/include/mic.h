#ifndef ATHENAOS_MIC_H
#define ATHENAOS_MIC_H

#include <Arduino.h>
#include "boards/board.h"

class Mic {
public:
    static bool begin();
    static bool isReady() { return _ready; }

private:
    static bool _ready;
};

#endif
