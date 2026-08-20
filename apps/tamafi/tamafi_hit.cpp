#include "tamafi_hit.h"

static bool inRect(int16_t x, int16_t y, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

bool TamafiHit::hitStats(int16_t x, int16_t y) {
    return inRect(x, y, STATS_X, STATS_Y, STATS_W, STATS_H);
}

bool TamafiHit::hitRestBtn(int16_t x, int16_t y) {
    return inRect(x, y, HUD_REST_X, HUD_BTN_Y, HUD_BTN_W, TFT_HEIGHT - HUD_BTN_Y);
}

bool TamafiHit::hitPlayBtn(int16_t x, int16_t y) {
    return inRect(x, y, HUD_PLAY_X, HUD_BTN_Y, HUD_BTN_W, TFT_HEIGHT - HUD_BTN_Y);
}
