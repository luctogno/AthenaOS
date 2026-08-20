#ifndef ATHENAOS_UI_FONT_H
#define ATHENAOS_UI_FONT_H

// UI_FONT_FAMILY comes from the board profile (board.h).

#if UI_FONT_FAMILY == UI_FONT_SANS
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold12pt7b.h"
#define UI_FONT_UI_GFX          FreeSans9pt7b
#define UI_FONT_TITLE_GFX       FreeSansBold12pt7b
#define UI_FONT_UI_BASELINE     13
#define UI_FONT_TITLE_BASELINE  16
#elif UI_FONT_FAMILY == UI_FONT_SERIF
#include "fonts/FreeSerif9pt7b.h"
#include "fonts/FreeSerifBold12pt7b.h"
#define UI_FONT_UI_GFX          FreeSerif9pt7b
#define UI_FONT_TITLE_GFX       FreeSerifBold12pt7b
#define UI_FONT_UI_BASELINE     13
#define UI_FONT_TITLE_BASELINE  16
#elif UI_FONT_FAMILY == UI_FONT_MONO
#include "fonts/FreeMono9pt7b.h"
#include "fonts/FreeMonoBold12pt7b.h"
#define UI_FONT_UI_GFX          FreeMono9pt7b
#define UI_FONT_TITLE_GFX       FreeMonoBold12pt7b
#define UI_FONT_UI_BASELINE     13
#define UI_FONT_TITLE_BASELINE  18
#endif

#endif
