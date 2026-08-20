#ifndef ATHENAOS_CLOCK_H
#define ATHENAOS_CLOCK_H

#include <Arduino.h>

class Clock {
public:
    static void begin();
    static void poll();
    static void applyTz();
    static void format(char *buf, size_t len);
    static bool hasTime();
};

#endif
