#include "icon.h"
#include "display.h"

void drawTamafiIcon(int16_t cx, int16_t cy, int16_t r) {
    Display::fillCircle(cx, cy, r, COLOR_BG);
    Display::fillCircle(cx, cy + 4, r - 8, COLOR_SOFT);
    Display::drawCircle(cx, cy + 4, r - 8, COLOR_PINK);
    Display::fillCircle(cx - 7, cy - 2, 5, COLOR_GOLD);
    Display::fillCircle(cx + 7, cy - 2, 5, COLOR_GOLD);
    Display::fillCircle(cx - 6, cy - 2, 2, COLOR_BG);
    Display::fillCircle(cx + 8, cy - 2, 2, COLOR_BG);
    Display::drawLine(cx - 10, cy + 8, cx + 12, cy - 4, COLOR_CYAN);
}
