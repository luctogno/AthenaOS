#include "icon.h"
#include "display.h"

void drawLauncherIcon(int16_t cx, int16_t cy, int16_t r) {
    Display::fillCircle(cx, cy, r, COLOR_BG);
    Display::fillCircle(cx, cy - 4, 12, COLOR_GOLD);
    Display::fillCircle(cx, cy + 14, 16, COLOR_SOFT);
    Display::drawCircle(cx, cy + 14, 16, COLOR_PINK);
}
