#ifndef ATHENAOS_APP_ICON_H
#define ATHENAOS_APP_ICON_H

#include <Arduino.h>

enum AppIcon {
    APP_ICON_DEFAULT = 0,
    APP_ICON_PET,
    APP_ICON_MIC
};

void drawAppIcon(int16_t cx, int16_t cy, int16_t r, AppIcon icon,
                 const uint16_t *bitmap, int16_t bw, int16_t bh);

#endif
