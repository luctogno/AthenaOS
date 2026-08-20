#ifndef ATHENAOS_BOARD_H
#define ATHENAOS_BOARD_H

#if defined(BOARD_WAVESHARE_AMOLED_18_V2)
#ifndef WAVESHARE_AMOLED_V2
#define WAVESHARE_AMOLED_V2 1
#endif
#include "waveshare_amoled_18.h"
#elif defined(BOARD_WAVESHARE_AMOLED_18)
#ifndef WAVESHARE_AMOLED_V2
#define WAVESHARE_AMOLED_V2 0
#endif
#include "waveshare_amoled_18.h"
#else
#error "Select a board env in platformio.ini (e.g. waveshare_amoled_18)"
#endif

#ifndef HOME_HOLD_MS
#define HOME_HOLD_MS            5000
#endif

#ifndef DISABLE_DEBUG
#define DEBUG_PRINT(x)           Serial.print(x)
#define DEBUG_PRINTLN(x)         Serial.println(x)
#define DEBUG_PRINTF(fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif

#endif
