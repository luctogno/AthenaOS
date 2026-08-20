#ifndef TAMAFI_CONFIG_H
#define TAMAFI_CONFIG_H

#include "boards/board.h"

#define BTN_BOOT_PIN        BTN1_PIN

#define UI_W                TFT_WIDTH
#define UI_H                TFT_HEIGHT
#define UI_OX               0
#define UI_OY               0

#define BG_W                368
#define BG_H                448
#define PET_W               115
#define PET_H               110
#define PET_SCALE           1.8f
#define EFFECT_W            100
#define EFFECT_H            95

#define HUD_TOP_H           56
#define HUD_BOT_H           80
#define STAGE_Y             0
#define STAGE_H             TFT_HEIGHT

#define STATS_X             26
#define STATS_Y             105
#define STATS_W             168
#define STATS_H             270

#define PET_DRAW_X          148
#define PET_DRAW_Y          190

#define HUD_BTN_Y           (TFT_HEIGHT - HUD_BTN_H)
#define HUD_BTN_H           56
#define HUD_BTN_W           (TFT_WIDTH / 2)
#define HUD_REST_X          0
#define HUD_PLAY_X          HUD_BTN_W

#define WIFI_SSID           "FritzHomeLT"
#define WIFI_PASSWORD       "homeLTogno55"
#define WEB_SERVER_PORT     80
#define STEPS_PER_FEED_BASE 10
#define HUNGER_PER_FEED     10
#define WIFI_NET_MAX        16

#endif
