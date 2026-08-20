#include <Arduino.h>
#include <WiFi.h>
#include <string.h>
#include "ui.h"
#include "ui_anim.h"
#include "StepDetector.h"
#include "settings.h"

int petPosX = PET_DRAW_X;
int petPosY = PET_DRAW_Y;

// Graphics headers
#include "StoneGolem.h"
#include "egg_hatch.h"
#include "effect.h"
#include "ui_bg.h"
#include "stat_icons.h"

static const int TFT_W = UI_W;
static const int TFT_H = UI_H;

// Hunting animation
static int huntFrame = 0;
static unsigned long lastHuntFrameTime = 0;
static const int HUNT_FRAME_DELAY = 300;   // adjust speed

// Idle sprite sets per stage (placeholder: same for all)
static const uint16_t* BABY_IDLE_FRAMES[4]  = { idle_1, idle_2, idle_3, idle_4 };
static const uint16_t* TEEN_IDLE_FRAMES[4]  = { idle_1, idle_2, idle_3, idle_4 };
static const uint16_t* ADULT_IDLE_FRAMES[4] = { idle_1, idle_2, idle_3, idle_4 };
static const uint16_t* ELDER_IDLE_FRAMES[4] = { idle_1, idle_2, idle_3, idle_4 };

// Egg frames
static const uint16_t* EGG_FRAMES[5] = {
    egg_hatch_1, egg_hatch_2, egg_hatch_3, egg_hatch_4, egg_hatch_5
};

static const uint16_t* EGG_IDLE_FRAMES[4] = {
    egg_hatch_11, egg_hatch_21, egg_hatch_31, egg_hatch_41
};

static const uint16_t* HUNGER_FRAMES[4] = {
    hunger1, hunger2, hunger3, hunger4
};

static const uint16_t* DEAD_FRAMES[3] = {
    dead_1, dead_2, dead_3
};

// HUNTING animation loop
static const uint16_t* ATTACK_FRAMES[3] = {
    attack_0, attack_1, attack_2
};


// Local UI state
static int idleFrameUi = 0;
static unsigned long lastIdleFrameUi = 0;
static int zzzFrame = 0;
static unsigned long lastZzzMs = 0;

static int eggIdleFrameUi = 0;
static unsigned long lastEggIdleTimeUi = 0;

static int hatchFrameUi = 0;
static unsigned long lastHatchFrameUi = 0;

static int deadFrameUi = 0;
static unsigned long lastDeadFrameUi = 0;

// Highlight animation states
static int menuHighlightY        = 30;
static int menuHighlightTargetY  = 30;
static unsigned long lastMenuAnimTime = 0;

static int ctlHighlightY         = 30;
static int ctlHighlightTargetY   = 30;
static unsigned long lastCtlAnim = 0;

static int setHighlightY         = 30;
static int setHighlightTargetY   = 30;
static unsigned long lastSetAnim = 0;

static const int MAIN_MENU_COUNT = 7;
static const int MENU_ROW_H = 36;
static const int MENU_TOP_Y = 52;
static const int LINE_H = 28;
static const int TEXT_PAD = 16;

// ---------------------------------------------------------------------------
// UNIVERSAL HIGHLIGHT ALIGNMENT
// ---------------------------------------------------------------------------
static int calcHighlightY(int rowIndex, int rowHeight, int topOffset) {
    return topOffset + rowIndex * rowHeight - 5;  // centered under text
}

static const char* moodTextLocal(Mood m) {
    switch (m) {
        case MOOD_HUNGRY:  return "HUNGRY";
        case MOOD_HAPPY:   return "HAPPY";
        case MOOD_CURIOUS: return "CURIOUS";
        case MOOD_BORED:   return "BORED";
        case MOOD_SICK:    return "SICK";
        case MOOD_EXCITED: return "EXCITED";
        case MOOD_CALM:    return "CALM";
    }
    return "?";
}

static const char* stageTextLocal(Stage s) {
    switch (s) {
        case STAGE_BABY:  return "BABY";
        case STAGE_TEEN:  return "TEEN";
        case STAGE_ADULT: return "ADULT";
        case STAGE_ELDER: return "ELDER";
    }
    return "?";
}

static const char* activityTextLocal(Activity a) {
    switch (a) {
        case ACT_HUNT:     return "HUNT";
        case ACT_DISCOVER: return "PLAY";
        case ACT_REST:     return "REST";
        default:           return "IDLE";
    }
}

static void drawHeader(const char* title) {
    fb.fillRect(0, 0, TFT_W, 36, TFT_BLACK);
    fb.drawLine(0, 36, TFT_W, 36, TFT_CYAN);
    fb.drawLine(0, 37, TFT_W, 37, TFT_MAGENTA);

    fb.fillRect(8, 10, 12, 12, TFT_WHITE);
    fb.fillRect(10, 12, 8, 8, TFT_BLACK);

    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);
    fb.setCursor(28, 10);
    fb.print(title);
}

static void drawBar(int x, int y, int w, int h, int value, uint16_t color) {
    fb.fillRect(x, y, w, h, TFT_WHITE);
    fb.drawRect(x, y, w, h, color);
    int fillWidth = (w - 2) * value / 100;
    if (fillWidth > 0) fb.fillRect(x + 1, y + 1, fillWidth, h - 2, color);

    char buf[4];
    snprintf(buf, sizeof(buf), "%d", value);
    uint8_t ts = h >= 18 ? 2 : 1;
    int th = 7 * ts;
    int tw = (int)strlen(buf) * 6 * ts;
    fb.setTextSize(ts);
    fb.setTextColor(TFT_BLACK);
    fb.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
    fb.print(buf);
}

static void drawUiBackground() {
    fb.pushImage(0, 0, BG_W, BG_H, uiBackground);
}

static void drawHudButton(int x, int y, int w, int h, const char *label, bool active) {
    uint16_t fill = active ? STATS_GRAY_HI : STATS_GRAY;
    fb.fillRect(x, y, w, h, fill);
    fb.drawRect(x, y, w, h, STATS_GRAY);
    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);
    int tw = (int)strlen(label) * 12;
    int tx = x + (w - tw) / 2;
    if (tx < x + 6) tx = x + 6;
    fb.setCursor(tx, y + (h - 16) / 2);
    fb.print(label);
}

static void drawBubble(int x, int y, bool selected) {
    if (selected) {
        fb.fillCircle(x, y, 6, TFT_WHITE);
        fb.fillCircle(x, y, 3, TFT_BLACK);
    } else {
        fb.drawCircle(x, y, 6, TFT_WHITE);
    }
}

static void drawMenuIcon(int iconIndex, int x, int y) {
    switch (iconIndex) {
        case 0: fb.drawRect(x, y+3, 5, 4, TFT_WHITE); fb.fillRect(x+1,y+4,3,2,TFT_WHITE); break;
        case 1: fb.drawLine(x+2,y+8,x+5,y+2,TFT_WHITE); fb.drawLine(x+8,y+8,x+5,y+2,TFT_WHITE); fb.fillRect(x+4,y+8,2,3,TFT_WHITE); break;
        case 2: fb.drawRect(x+1,y+2,8,6,TFT_WHITE); fb.drawPixel(x,y+3,TFT_WHITE); fb.drawPixel(x+9,y+3,TFT_WHITE); break;
        case 3: fb.drawLine(x+1,y+3,x+9,y+3,TFT_WHITE); fb.fillRect(x+3,y+2,3,3,TFT_WHITE); break;
        case 4: fb.drawCircle(x+5,y+5,3,TFT_WHITE); break;
        case 5: fb.fillRect(x+4,y+2,2,2,TFT_WHITE); break;
        case 6: fb.drawLine(x+8,y+4,x+2,y+4,TFT_WHITE); fb.drawLine(x+2,y+4,x+4,y+2,TFT_WHITE); break;
    }
}

static void animateSelector(int &pos, int &target, unsigned long &lastTick) {
    unsigned long now = millis();
    if (now - lastTick < MENU_ANIM_INTERVAL) return;
    lastTick = now;
    if (pos == target) return;

    int diff = target - pos;
    int step = (diff > 0) ? MENU_ANIM_STEP : -MENU_ANIM_STEP;
    if (abs(diff) < MENU_ANIM_STEP) pos = target;
    else pos += step;
}

static const uint16_t** currentIdleSet() {
    switch (petStage) {
        case STAGE_BABY:  return BABY_IDLE_FRAMES;
        case STAGE_TEEN:  return TEEN_IDLE_FRAMES;
        case STAGE_ADULT: return ADULT_IDLE_FRAMES;
        case STAGE_ELDER: return ELDER_IDLE_FRAMES;
    }
    return BABY_IDLE_FRAMES;
}

// ---------------------------------------------------------------------------
// BOOT SCREEN
// ---------------------------------------------------------------------------
static void screenBoot() {
    fb.fillSprite(TFT_BLACK);
    drawHeader("TamaFi");

    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);
    fb.setCursor(TEXT_PAD, 80);
    fb.print("WiFi-fed pet");

    fb.setCursor(TEXT_PAD, 130);
    fb.print("BOOT / touch");
    fb.setCursor(TEXT_PAD, 158);
    fb.print("to start");

    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// HATCH SCREEN (Idle egg → OK → hatch → home)
// ---------------------------------------------------------------------------
static void screenHatch() {
    fb.fillSprite(TFT_BLACK);
    drawUiBackground();
    unsigned long now = millis();
    int eggX = PET_DRAW_X;
    int eggY = PET_DRAW_Y;

    if (!hasHatchedOnce && !hatchTriggered) {
        if (now - lastEggIdleTimeUi >= EGG_IDLE_DELAY) {
            lastEggIdleTimeUi = now;
            eggIdleFrameUi = (eggIdleFrameUi + 1) % 4;
        }

        petSprite.pushImage(0, 0, PET_W, PET_H, EGG_IDLE_FRAMES[eggIdleFrameUi]);
        petSprite.pushToSpriteScaled(&fb, eggX, eggY, TFT_WHITE, PET_SCALE);

        fb.setTextSize(2);
        fb.setTextColor(TFT_WHITE);
        fb.setCursor(TEXT_PAD, TFT_H - 40);
        fb.print("Tap to hatch");

        fb.pushSprite(0, 0);
        return;
    }

    if (!hasHatchedOnce && hatchTriggered) {
      if (hatchFrameUi == 0) {
        sndHatch();
        }
        if (now - lastHatchFrameUi >= HATCH_DELAY) {
            lastHatchFrameUi = now;

            if (hatchFrameUi < 4) hatchFrameUi++;
            else {
                hasHatchedOnce = true;
                hatchTriggered = false;
                hatchFrameUi   = 0;
                currentScreen  = SCREEN_HOME;
                uiOnScreenChange(currentScreen);
                return;
            }
        }

        petSprite.pushImage(0, 0, PET_W, PET_H, EGG_FRAMES[hatchFrameUi]);
        petSprite.pushToSpriteScaled(&fb, eggX, eggY, TFT_WHITE, PET_SCALE);

        fb.pushSprite(0, 0);
        return;
    }

    currentScreen = SCREEN_HOME;
    uiOnScreenChange(currentScreen);
}

// ---------------------------------------------------------------------------
// HOME SCREEN
// ---------------------------------------------------------------------------

static void drawLabeledValue(int x, int y, const char *label, const char *value) {
    fb.setTextSize(2);
    fb.setTextColor(TFT_BLACK);
    fb.setCursor(x, y);
    fb.print(label);
    int lw = (int)strlen(label) * 12;
    fb.drawLine(x, y + 17, x + lw, y + 17, TFT_BLACK);
    fb.setCursor(x, y + 22);
    fb.print(value);
}

static void blitStatIcon(int x, int y, const uint16_t *icon) {
    for (int j = 0; j < STAT_ICON_H; j++) {
        for (int i = 0; i < STAT_ICON_W; i++) {
            uint16_t c = icon[j * STAT_ICON_W + i];
            if (c == TFT_WHITE) continue;
            fb.drawPixel(x + i, y + j, c);
        }
    }
}

static void drawStatsBlock() {
    int x = STATS_X, y = STATS_Y, h = 20, gap = 14;
    int rowH = STAT_ICON_H + gap;
    int barX = x + STAT_ICON_W + 4;
    int barW = STATS_W - STAT_ICON_W - 4;
    int barYoff = (STAT_ICON_H - h) / 2;

    blitStatIcon(x, y, iconHunger);
    drawBar(barX, y + barYoff, barW, h, pet.hunger, MAT_RED);

    blitStatIcon(x, y + rowH, iconHappy);
    drawBar(barX, y + rowH + barYoff, barW, h, pet.happiness, MAT_YELLOW);

    blitStatIcon(x, y + 2 * rowH, iconHealth);
    drawBar(barX, y + 2 * rowH + barYoff, barW, h, pet.health, MAT_GREEN);

    int ty = y + 3 * rowH - gap + 8;
    int tx = x + 5;
    drawLabeledValue(tx, ty, "Mood:", moodTextLocal(currentMood));
    drawLabeledValue(tx, ty + 46, "Stage:", stageTextLocal(petStage));
    drawLabeledValue(tx, ty + 92, "Status:", activityTextLocal(currentActivity));
}

static void drawHomeHud() {
    int missing = StepDetector::stepsUntilFeed();
    fb.setTextSize(2);
    fb.setTextColor(TFT_BLACK);
    fb.setCursor(50, 12);
    fb.print(missing);
    fb.print(" steps to feed");

    bool resting = (currentActivity == ACT_REST && restPhase != REST_NONE);
    bool playing = (currentActivity == ACT_DISCOVER);
    drawHudButton(HUD_REST_X, HUD_BTN_Y, HUD_BTN_W, HUD_BTN_H, "REST", resting);
    drawHudButton(HUD_PLAY_X, HUD_BTN_Y, HUD_BTN_W, HUD_BTN_H, "PLAY", playing);
}

static void drawSleepingGolem(unsigned long now) {
    if (now - lastZzzMs >= 320) {
        lastZzzMs = now;
        zzzFrame = (zzzFrame + 1) % 4;
    }

    int frameIdx = 0;
    if (restPhase == REST_ENTER || restPhase == REST_WAKE) {
        frameIdx = constrain(restFrameIndex, 0, 4);
    }

    petSprite.pushImage(0, 0, PET_W, PET_H, EGG_FRAMES[frameIdx]);
    petSprite.pushToSpriteScaled(&fb, petPosX, petPosY, TFT_WHITE, PET_SCALE);

    int zw = (int)(PET_W * PET_SCALE + 0.5f);
    int zx = petPosX + zw - 28;
    int zy = petPosY - 6;
    fb.setTextSize(2);
    fb.setTextColor(TFT_BLACK);
    if (zzzFrame >= 1) { fb.setCursor(zx, zy - 8); fb.print("z"); }
    if (zzzFrame >= 2) { fb.setCursor(zx + 12, zy - 22); fb.print("z"); }
    if (zzzFrame >= 3) { fb.setCursor(zx + 24, zy - 38); fb.print("Z"); }
}

static void screenHome() {
    fb.fillSprite(TFT_BLACK);
    drawUiBackground();

    unsigned long now = millis();

    if (currentActivity == ACT_REST && restPhase != REST_NONE) {
        drawSleepingGolem(now);
        drawStatsBlock();
        drawHomeHud();
        fb.pushSprite(0, 0);
        return;
    }

    if (currentActivity == ACT_HUNT) {
        if (now - lastHuntFrameTime >= HUNT_FRAME_DELAY) {
            lastHuntFrameTime = now;
            huntFrame = (huntFrame + 1) % 3;
        }

        petSprite.pushImage(0, 0, PET_W, PET_H, ATTACK_FRAMES[huntFrame]);
        petSprite.pushToSpriteScaled(&fb, petPosX, petPosY, TFT_WHITE, PET_SCALE);
        drawStatsBlock();
        drawHomeHud();
        fb.pushSprite(0, 0);
        return;
    }

    int idleSpeed = IDLE_BASE_DELAY;
    if (currentMood == MOOD_EXCITED) idleSpeed = IDLE_FAST_DELAY;
    if (currentMood == MOOD_BORED || currentMood == MOOD_SICK) idleSpeed = IDLE_SLOW_DELAY;

    if (now - lastIdleFrameUi >= (unsigned long)idleSpeed) {
        lastIdleFrameUi = now;
        idleFrameUi = (idleFrameUi + 1) % 4;
    }

    const uint16_t** idleSet = currentIdleSet();
    petSprite.pushImage(0, 0, PET_W, PET_H, idleSet[idleFrameUi]);
    petSprite.pushToSpriteScaled(&fb, petPosX, petPosY, TFT_WHITE, PET_SCALE);

    drawStatsBlock();

    if (hungerEffectActive) {
        effectSprite.pushImage(0, 0, EFFECT_W, EFFECT_H, HUNGER_FRAMES[hungerEffectFrame]);
        effectSprite.pushToSpriteScaled(&fb, petPosX, petPosY, TFT_WHITE, PET_SCALE);
    }

    drawHomeHud();
    fb.pushSprite(0, 0);
}


// ---------------------------------------------------------------------------
// MAIN MENU
// ---------------------------------------------------------------------------
static void drawFooter(const char *text) {
    fb.setTextSize(2);
    fb.setTextColor(TFT_DARKGREY);
    fb.setCursor(TEXT_PAD, TFT_H - 32);
    fb.print(text);
}

static void screenMenu(int mainMenuIndex) {
    fb.fillSprite(TFT_BLACK);
    drawHeader("Menu");

    animateSelector(menuHighlightY, menuHighlightTargetY, lastMenuAnimTime);

    fb.fillRect(12, menuHighlightY, TFT_W - 24, 30, TFT_DARKGREY);
    fb.drawRect(12, menuHighlightY, TFT_W - 24, 30, TFT_CYAN);

    const char* items[] = {
        "Pet Status",
        "Environment",
        "System Info",
        "Controls",
        "Settings",
        "Diagnostics",
        "Back"
    };

    fb.setTextSize(2);
    for (int i = 0; i < MAIN_MENU_COUNT; i++) {
        int y = MENU_TOP_Y + i * MENU_ROW_H;
        drawMenuIcon(i, 20, y + 4);
        fb.setCursor(48, y + 6);
        fb.setTextColor(i == mainMenuIndex ? TFT_YELLOW : TFT_WHITE);
        fb.print(items[i]);
    }

    drawFooter("UP / DOWN / OK");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// PET STATUS
// ---------------------------------------------------------------------------
static void screenPetStatus() {
    fb.fillSprite(TFT_BLACK);
    drawHeader("Pet Status");
    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);

    int y = 52;
    fb.setCursor(TEXT_PAD, y); fb.print("Stage "); fb.print(stageTextLocal(petStage));
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print("Age ");
    fb.print(pet.ageDays); fb.print("d ");
    fb.print(pet.ageHours); fb.print("h ");
    fb.print(pet.ageMinutes); fb.print("m");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Hunger "); fb.print(pet.hunger); fb.print("%");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Happy  "); fb.print(pet.happiness); fb.print("%");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Health "); fb.print(pet.health); fb.print("%");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Mood "); fb.print(moodTextLocal(currentMood));
    y += LINE_H + 8;
    fb.setCursor(TEXT_PAD, y); fb.print("Curiosity "); fb.print((int)traitCuriosity);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Activity  "); fb.print((int)traitActivity);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Stress    "); fb.print((int)traitStress);

    drawFooter("OK = Back");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// ENVIRONMENT
// ---------------------------------------------------------------------------
static void screenEnvironment() {
    fb.fillSprite(TFT_BLACK);
    drawHeader("Environment");
    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);

    int y = 56;
    fb.setCursor(TEXT_PAD, y); fb.print("Networks "); fb.print(wifiStats.netCount);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Strong   "); fb.print(wifiStats.strongCount);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Hidden   "); fb.print(wifiStats.hiddenCount);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Open     "); fb.print(wifiStats.openCount);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("WPA      "); fb.print(wifiStats.wpaCount);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Avg RSSI "); fb.print(wifiStats.avgRSSI);

    drawFooter("OK = Back");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// SYSTEM INFO
// ---------------------------------------------------------------------------
static void screenSysInfo() {
    fb.fillSprite(TFT_BLACK);
    drawHeader("System");
    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);

    int y = 56;
    fb.setCursor(TEXT_PAD, y); fb.print("FW 2.0");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("ESP32-S3");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Heap "); fb.print(ESP.getFreeHeap() / 1024); fb.print(" KB");
    y += LINE_H;

    unsigned long s = millis() / 1000;
    unsigned long m = s / 60;
    unsigned long h = m / 60;
    s %= 60; m %= 60;
    fb.setCursor(TEXT_PAD, y);
    fb.printf("Up %02lu:%02lu:%02lu", h, m, s);
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print(wifiScanInProgress ? "Scan run" : "Scan idle");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print(WiFi.SSID().length() ? WiFi.SSID() : "-");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print(WiFi.isConnected() ? WiFi.localIP().toString() : "-");

    drawFooter("OK = Back");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// CONTROLS MENU
// ---------------------------------------------------------------------------
static void screenControls(int controlsIndex) {
    fb.fillSprite(TFT_BLACK);
    drawHeader("Controls");

    animateSelector(ctlHighlightY, ctlHighlightTargetY, lastCtlAnim);
    fb.fillRect(12, ctlHighlightY, TFT_W - 24, 30, TFT_DARKGREY);
    fb.drawRect(12, ctlHighlightY, TFT_W - 24, 30, TFT_CYAN);

    const char* labels[] = {
        "Screen bri",
        "LED bri",
        "Sound",
        "NeoPixels",
        "Back"
    };

    fb.setTextSize(2);
    for (int i = 0; i < 5; i++) {
        int y = MENU_TOP_Y + i * MENU_ROW_H;
        drawBubble(22, y + 12, i == controlsIndex);
        fb.setCursor(48, y + 6);
        fb.setTextColor(i == controlsIndex ? TFT_YELLOW : TFT_WHITE);
        fb.print(labels[i]);

        fb.setCursor(220, y + 6);
        fb.setTextColor(TFT_CYAN);
        switch (i) {
            case 0: {
                uint8_t b = Settings::brightness();
                fb.print(b == 0 ? "Low" : b == 1 ? "Mid" : "High");
                break;
            }
            case 1:
                fb.print(ledBrightnessIndex==0?"Low":ledBrightnessIndex==1?"Mid":"High");
                break;
            case 2:
                fb.print(soundEnabled?"On":"Off");
                break;
            case 3:
                fb.print(neoPixelsEnabled?"On":"Off");
                break;
        }
    }

    drawFooter("OK = Select");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// SETTINGS MENU
// ---------------------------------------------------------------------------
static void screenSettings(int settingsMenuIndex) {
    fb.fillSprite(TFT_BLACK);
    drawHeader("Settings");

    animateSelector(setHighlightY, setHighlightTargetY, lastSetAnim);
    fb.fillRect(12, setHighlightY, TFT_W - 24, 30, TFT_DARKGREY);
    fb.drawRect(12, setHighlightY, TFT_W - 24, 30, TFT_CYAN);

    const char* labels[] = {
        "Theme",
        "Auto Sleep",
        "Auto Save",
        "Step feed",
        "Reset Pet",
        "Reset All",
        "Back"
    };

    fb.setTextSize(2);
    for (int i = 0; i < 7; i++) {
        int y = MENU_TOP_Y + i * MENU_ROW_H;
        drawBubble(22, y + 12, i == settingsMenuIndex);
        fb.setCursor(48, y + 6);
        fb.setTextColor(i == settingsMenuIndex ? TFT_YELLOW : TFT_WHITE);
        fb.print(labels[i]);

        fb.setCursor(230, y + 6);
        fb.setTextColor(TFT_CYAN);
        switch (i) {
            case 0: fb.print("Pixel"); break;
            case 1: fb.print(autoSleep?"On":"Off"); break;
            case 2: fb.print(autoSaveMs/1000); fb.print("s"); break;
            case 3: fb.print(StepDetector::multiplier()); fb.print("x"); break;
        }
    }

    drawFooter("OK = Select");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// DIAGNOSTICS
// ---------------------------------------------------------------------------
static void screenDiagnostics() {
    fb.fillSprite(TFT_BLACK);
    drawHeader("Diagnostics");
    fb.setTextSize(2);
    fb.setTextColor(TFT_WHITE);

    int y = 56;
    fb.setCursor(TEXT_PAD, y); fb.print(activityTextLocal(currentActivity));
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y); fb.print("Mood "); fb.print(moodTextLocal(currentMood));
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print(restPhase==REST_ENTER?"Rest ENTER":
             restPhase==REST_DEEP ?"Rest DEEP":
             restPhase==REST_WAKE ?"Rest WAKE":"Rest none");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print(wifiScanInProgress?"Scan run":"Scan idle");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print(StepDetector::imuReady() ? "IMU ok" : "IMU none");
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print("Steps ");
    fb.print(StepDetector::total());
    y += LINE_H;
    fb.setCursor(TEXT_PAD, y);
    fb.print("Rem ");
    fb.print(StepDetector::remainder());

    drawFooter("OK = Back");
    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// GAME OVER
// ---------------------------------------------------------------------------
static void screenGameOver() {
    unsigned long now = millis();
    if (now - lastDeadFrameUi >= DEAD_DELAY) {
        lastDeadFrameUi = now;
        deadFrameUi++;
        if (deadFrameUi > 2) deadFrameUi = 2;
    }

    drawUiBackground();

    petSprite.pushImage(0, 0, PET_W, PET_H, DEAD_FRAMES[deadFrameUi]);
    petSprite.pushToSpriteScaled(&fb, petPosX, petPosY, TFT_WHITE, PET_SCALE);

    //fb.setCursor(10, 200);
    //fb.setTextColor(TFT_WHITE);
    //fb.print("OK = Restart");

    fb.pushSprite(0, 0);
}

// ---------------------------------------------------------------------------
// PUBLIC UI API
// ---------------------------------------------------------------------------
void uiInit() {
    idleFrameUi = 0;
    lastIdleFrameUi = millis();

    eggIdleFrameUi = 0;
    lastEggIdleTimeUi = millis();

    hatchFrameUi = 0;
    lastHatchFrameUi = millis();

    deadFrameUi = 0;
    lastDeadFrameUi = millis();
}

void uiOnScreenChange(Screen newScreen) {
    if (newScreen == SCREEN_MENU) {
        menuHighlightY = menuHighlightTargetY = calcHighlightY(mainMenuIndex, MENU_ROW_H, MENU_TOP_Y);
    }
    if (newScreen == SCREEN_CONTROLS) {
        ctlHighlightY  = ctlHighlightTargetY = calcHighlightY(controlsIndex, MENU_ROW_H, MENU_TOP_Y);
    }
    if (newScreen == SCREEN_SETTINGS) {
        setHighlightY  = setHighlightTargetY = calcHighlightY(settingsMenuIndex, MENU_ROW_H, MENU_TOP_Y);
    }
    if (newScreen == SCREEN_HATCH) {
        eggIdleFrameUi = hatchFrameUi = 0;
    }
}

void uiDrawScreen(Screen screen,
                  int mainMenuIdx,
                  int controlsIdx,
                  int settingsIdx)
{
    if (screen == SCREEN_MENU) {
        menuHighlightTargetY = calcHighlightY(mainMenuIdx, MENU_ROW_H, MENU_TOP_Y);
    }
    if (screen == SCREEN_CONTROLS) {
        ctlHighlightTargetY = calcHighlightY(controlsIdx, MENU_ROW_H, MENU_TOP_Y);
    }
    if (screen == SCREEN_SETTINGS) {
        setHighlightTargetY = calcHighlightY(settingsMenuIndex, MENU_ROW_H, MENU_TOP_Y);
    }

    switch (screen) {
        case SCREEN_BOOT:        screenBoot(); break;
        case SCREEN_HATCH:       screenHatch(); break;
        case SCREEN_HOME:        screenHome(); break;
        case SCREEN_MENU:        screenMenu(mainMenuIdx); break;
        case SCREEN_PET_STATUS:  screenPetStatus(); break;
        case SCREEN_ENVIRONMENT: screenEnvironment(); break;
        case SCREEN_SYSINFO:     screenSysInfo(); break;
        case SCREEN_CONTROLS:    screenControls(controlsIdx); break;
        case SCREEN_SETTINGS:    screenSettings(settingsIdx); break;
        case SCREEN_DIAGNOSTICS: screenDiagnostics(); break;
        case SCREEN_GAMEOVER:    screenGameOver(); break;
    }
}
