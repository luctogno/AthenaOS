#include "icon.h"
#include "display.h"

// Same mark as TamaFi Settings: apps/tamafi/ui.cpp drawMenuIcon case 4.
void drawSettingsIcon(int16_t cx, int16_t cy, int16_t r) {
    Display::fillCircle(cx, cy, r, COLOR_BG);
    int16_t outer = r - 8;
    if (outer < 8) outer = 8;
    Display::drawCircle(cx, cy, outer, COLOR_FG);
    Display::drawCircle(cx, cy, outer - 1, COLOR_FG);
}
