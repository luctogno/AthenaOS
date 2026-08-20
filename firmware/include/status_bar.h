#ifndef ATHENAOS_STATUS_BAR_H
#define ATHENAOS_STATUS_BAR_H

#include <Arduino.h>

class StatusBar {
public:
    static constexpr int16_t HEIGHT = 52;
    static int16_t height() { return HEIGHT; }
    static void draw(const char *title);
    static void tick();
    static void hide();
};

#endif
