#include "app_icon.h"
#include "display.h"

void drawAppIcon(int16_t cx, int16_t cy, int16_t r, AppIcon icon,
                 const uint16_t *bitmap, int16_t bw, int16_t bh) {
    if (bitmap && bw > 0 && bh > 0) {
        Display::blit(cx - bw / 2, cy - bh / 2, bitmap, bw, bh);
        return;
    }

    Display::fillCircle(cx, cy, r, COLOR_BG);

    switch (icon) {
    case APP_ICON_PET:
        Display::fillCircle(cx, cy + 4, r - 8, 0x1C4C);
        Display::fillCircle(cx - 7, cy - 2, 5, COLOR_FG);
        Display::fillCircle(cx + 7, cy - 2, 5, COLOR_FG);
        Display::fillCircle(cx - 6, cy - 2, 2, 0x0000);
        Display::fillCircle(cx + 8, cy - 2, 2, 0x0000);
        Display::fillTriangle(cx, cy + 2, cx - 5, cy + 10, cx + 5, cy + 10, COLOR_GOLD);
        break;
    case APP_ICON_MIC:
        Display::fillRoundRect(cx - 7, cy - 14, 14, 20, 7, COLOR_ACCENT);
        Display::drawLine(cx, cy + 6, cx, cy + 14, COLOR_ACCENT);
        Display::drawLine(cx - 8, cy + 14, cx + 8, cy + 14, COLOR_ACCENT);
        Display::drawCircle(cx, cy - 4, r - 10, COLOR_MUTED);
        break;
    case APP_ICON_GEAR:
        Display::fillCircle(cx, cy, r - 6, COLOR_SECOND);
        Display::fillRect(cx - 4, cy - (r - 4), 8, (r - 4) * 2, COLOR_SECOND);
        Display::fillRect(cx - (r - 4), cy - 4, (r - 4) * 2, 8, COLOR_SECOND);
        Display::fillCircle(cx, cy, r - 14, COLOR_BG);
        break;
    case APP_ICON_DEFAULT:
    default:
        Display::fillCircle(cx, cy - 4, 12, COLOR_GOLD);
        Display::fillCircle(cx, cy + 14, 16, 0x1C4C);
        break;
    }
}
