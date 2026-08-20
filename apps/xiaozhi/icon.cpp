#include "icon.h"
#include "display.h"

void drawXiaozhiIcon(int16_t cx, int16_t cy, int16_t r) {
    Display::fillCircle(cx, cy, r, COLOR_BG);
    Display::fillRoundRect(cx - 7, cy - 14, 14, 20, 7, COLOR_ACCENT);
    Display::drawLine(cx, cy + 6, cx, cy + 14, COLOR_ACCENT);
    Display::drawLine(cx - 8, cy + 14, cx + 8, cy + 14, COLOR_ACCENT);
    Display::drawCircle(cx, cy - 4, r - 10, COLOR_MUTED);
}
