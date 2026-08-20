#ifndef ATHENAOS_SPLASH_H
#define ATHENAOS_SPLASH_H

#include <Arduino.h>

class Splash {
public:
    static void begin();
    static bool update();
};

#endif
