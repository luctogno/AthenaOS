#include "i18n.h"
#include "settings.h"

static const char *const STR_EN[I18N_COUNT] = {
    "Settings",
    "Volume",
    "WiFi",
    "Off",
    "Connected",
    "Not connected",
    "Radio off",
    "Network",
    "Setup hotspot",
    "1. Join this WiFi",
    "2. Open on phone:",
    "3. Pick home WiFi",
    "Hotspot on",
    "Start hotspot",
    "Stop hotspot",
    "Start hotspot to set WiFi",
    "Restart",
    "Power off",
    "Restart?",
    "Device will reboot",
    "Power off?",
    "Hold PWR to turn on",
    "WiFi connected",
    "Restart to apply?",
    "Yes",
    "No",
    "NTP ok",
    "NTP...",
    "No apps",
    "Tap to open",
    "Hold BOOT %lus to go home",
    "Factory reset",
    "Reset all?",
    "Erases WiFi, volume and app data",
    "Language",
    "Sound",
    "Mic",
    "Play",
    "Play...",
    "Test",
    "Listen",
    "N/A",
};

static const char *const STR_IT[I18N_COUNT] = {
    "Impostazioni",
    "Volume",
    "WiFi",
    "Off",
    "Connesso",
    "Non connesso",
    "Radio spenta",
    "Rete",
    "Setup hotspot",
    "1. Collegati a questo WiFi",
    "2. Apri sul telefono:",
    "3. Scegli il WiFi di casa",
    "Hotspot attivo",
    "Avvia hotspot",
    "Spegni hotspot",
    "Avvia l'hotspot per il WiFi",
    "Riavvia",
    "Spegni",
    "Riavviare?",
    "Il device si riavvia",
    "Spegnere?",
    "Tieni PWR per accendere",
    "WiFi connesso",
    "Riavvia per applicare?",
    "Si",
    "No",
    "NTP ok",
    "NTP...",
    "Nessuna app",
    "Tocca per aprire",
    "Tieni BOOT %lus per home",
    "Ripristino",
    "Ripristinare?",
    "Cancella WiFi, volume e dati app",
    "Lingua",
    "Suono",
    "Mic",
    "Play",
    "Play...",
    "Test",
    "Ascolto",
    "N/D",
};

struct LangPack {
    const char *id;
    const char *label;
    const char *const *str;
};

static const LangPack LANGS[] = {
    {"en", "English", STR_EN},
    {"it", "Italiano", STR_IT},
};
static const int LANG_COUNT = (int)(sizeof(LANGS) / sizeof(LANGS[0]));

static bool langEq(const char *a, const char *b) {
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (char)(*b + 32) : *b;
        if (ca != cb) return false;
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

int I18n::langCount() {
    return LANG_COUNT;
}

const char *I18n::langId(int index) {
    if (index < 0 || index >= LANG_COUNT) return LANGS[0].id;
    return LANGS[index].id;
}

const char *I18n::langLabel(int index) {
    if (index < 0 || index >= LANG_COUNT) return LANGS[0].label;
    return LANGS[index].label;
}

int I18n::langIndex(const char *id) {
    for (int i = 0; i < LANG_COUNT; i++) {
        if (langEq(LANGS[i].id, id)) return i;
    }
    return 0;
}

const char *I18n::t(I18nId id) {
    if (id >= I18N_COUNT) return "";
    int i = langIndex(Settings::lang());
    const char *s = LANGS[i].str[id];
    return s ? s : LANGS[0].str[id];
}
