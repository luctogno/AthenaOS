#ifndef ATHENAOS_BOARD_H
#define ATHENAOS_BOARD_H

#define UI_FONT_GLCD            0
#define UI_FONT_SANS            1
#define UI_FONT_SERIF           2
#define UI_FONT_MONO            3

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

#ifndef COLOR_GOLD
#define COLOR_GOLD              0xFEA0
#endif
#ifndef COLOR_SOFT
#define COLOR_SOFT              0xFDBB
#endif
#ifndef COLOR_PINK
#define COLOR_PINK              0xFB56
#endif
#ifndef COLOR_CYAN
#define COLOR_CYAN              0x07FF
#endif
#ifndef COLOR_MAIN
#define COLOR_MAIN              COLOR_GOLD
#endif
#ifndef COLOR_SECOND
#define COLOR_SECOND            COLOR_CYAN
#endif
#ifndef COLOR_ACCENT
#define COLOR_ACCENT            COLOR_PINK
#endif
#ifndef COLOR_BG
#define COLOR_BG                0x0843
#endif
#ifndef COLOR_FG
#define COLOR_FG                0xFFFF
#endif
#ifndef COLOR_PANEL
#define COLOR_PANEL             0x2889
#endif
#ifndef COLOR_MUTED
#define COLOR_MUTED             0x8BD3
#endif
#ifndef COLOR_ERROR
#define COLOR_ERROR             0xF800
#endif

#ifndef UI_FONT_FAMILY
#define UI_FONT_FAMILY          UI_FONT_SANS
#endif
#ifndef WEB_CONSOLE_PORT
#define WEB_CONSOLE_PORT        8080
#endif
#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID            "AthenaOS"
#endif
#ifndef I2S_PA_PIN
#define I2S_PA_PIN              PIN_NONE
#endif
#ifndef I2S_SAMPLE_RATE
#define I2S_SAMPLE_RATE         16000
#endif

#ifndef DISABLE_DEBUG
#include "log_buffer.h"
#define DEBUG_PRINT(x)           LogBuffer::out().print(x)
#define DEBUG_PRINTLN(x)         LogBuffer::out().println(x)
#define DEBUG_PRINTF(fmt, ...)   LogBuffer::out().printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif

#endif
