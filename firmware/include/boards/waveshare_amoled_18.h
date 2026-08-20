#ifndef ATHENAOS_BOARD_WAVESHARE_AMOLED_18_H
#define ATHENAOS_BOARD_WAVESHARE_AMOLED_18_H

// Waveshare ESP32-S3-Touch-AMOLED-1.8
// Pin map from TamaFi + ESP32-TamaPetchi config_v2.h
// PIN_NONE / HAS_*=0 = feature off for this board (and for future boards).

#define PIN_NONE                (-1)

#define BOARD_NAME              "Waveshare AMOLED 1.8"
#define SERIAL_BAUD             115200

#ifndef WAVESHARE_AMOLED_V2
#define WAVESHARE_AMOLED_V2     0
#endif

// --- Display (SH8601 / CO5300 via QSPI) ---
#define HAS_DISPLAY             1
#define SCREEN_WIDTH            368
#define SCREEN_HEIGHT           448
#define TFT_WIDTH               SCREEN_WIDTH
#define TFT_HEIGHT              SCREEN_HEIGHT
#define TFT_PIN_CS              12
#define TFT_PIN_SCLK            11
#define TFT_PIN_D0              4
#define TFT_PIN_D1              5
#define TFT_PIN_D2              6
#define TFT_PIN_D3              7
#define TFT_PIN_RST             PIN_NONE
#define TFT_PIN_BL              PIN_NONE

// --- I2C (touch, IMU, codec, PMU, RTC, expander) ---
#define I2C_SDA_PIN             15
#define I2C_SCL_PIN             14

// --- Touch (FT3168 V1 / CST820 V2) ---
#define HAS_TOUCH               1
#if WAVESHARE_AMOLED_V2
#define TOUCH_I2C_ADDR          0x15
#else
#define TOUCH_I2C_ADDR          0x38
#endif
#define TOUCH_PIN_INT           21

// --- Buttons (GPIO; PIN_NONE = unused) ---
#define HAS_BUTTONS             1
#define BTN1_PIN                0       // BOOT, active-low
#define BTN2_PIN                PIN_NONE
#define BTN3_PIN                PIN_NONE
#define HOME_HOLD_MS            5000    // hold BOOT to return to launcher; 0 = off

// --- IO expander (PWR is not a native GPIO) ---
#define HAS_PWR_EXPANDER        1
#define TCA9554_ADDR            0x20
#define TCA9554_EXIO_PWR        4       // active-high; do not hold >6s

// --- IMU (QMI8658) ---
#define HAS_IMU                 1
#define IMU_I2C_ADDR            0x6A    // probe also 0x6B
#define IMU_PIN_INT             PIN_NONE

// --- Pedometer (software on IMU, not a separate chip) ---
#define HAS_PEDOMETER           1
#define STEP_PEAK_G             1.18f
#define STEP_VALLEY_G           0.92f
#define STEP_MIN_INTERVAL_MS    280

// --- Audio / mic (ES8311 codec; driver stub for now) ---
#define HAS_AUDIO               1
#define HAS_MIC                 1
#define AUDIO_I2C_ADDR          0x18
#define I2S_MCLK_PIN            16
#define I2S_BCLK_PIN            9
#define I2S_LRC_PIN             45
#define I2S_DIN_PIN             8       // MCU -> codec (speaker)
#define I2S_DOUT_PIN            10      // codec -> MCU (mic)
#define I2S_SAMPLE_RATE         44100
#define I2S_BITS_PER_SAMPLE     16

// --- BLE (on-chip radio, no GPIO) ---
#define HAS_BLE                 1
#define HAS_CLASSIC_BT          0
#define BLE_DEVICE_NAME         "AthenaOS"

// --- NFC (not onboard the 1.8; slot for a future board / external PN532) ---
#define HAS_NFC                 0
#define NFC_I2C_ADDR            0x24
#define NFC_PIN_IRQ             PIN_NONE
#define NFC_PIN_RST             PIN_NONE

// --- PMU (AXP2101; driver later) ---
#define HAS_PMU                 1
#define PMU_I2C_ADDR            0x34
#define BATTERY_ADC_PIN         PIN_NONE

// --- RTC (PCF85063; driver later) ---
#define HAS_RTC                 1
#define RTC_I2C_ADDR            0x51

// --- SDMMC (driver later) ---
#define HAS_SD                  1
#define SD_PIN_CMD              1
#define SD_PIN_CLK              2
#define SD_PIN_D0               3
#define SD_PIN_D1               PIN_NONE
#define SD_PIN_D2               PIN_NONE
#define SD_PIN_D3               PIN_NONE

// --- Theme (RGB565) ---
// #FFD700 gold, #FFB6D9 soft pink, #FF69B4 hot pink, #00FFFF cyan, #FFFFFF
// bg #0a0a1a, panel dark purple
#define COLOR_GOLD              0xFEA0
#define COLOR_SOFT              0xFDBB
#define COLOR_PINK              0xFB56
#define COLOR_CYAN              0x07FF
#define COLOR_MAIN              COLOR_GOLD
#define COLOR_SECOND            COLOR_CYAN
#define COLOR_ACCENT            COLOR_PINK
#define COLOR_BG                0x0843
#define COLOR_FG                0xFFFF
#define COLOR_PANEL             0x2889
#define COLOR_MUTED             0x8BD3
#define COLOR_ERROR             0xF800
#define UI_FONT_FAMILY          UI_FONT_SANS  // GLCD | SANS | SERIF | MONO
#define WEB_CONSOLE_PORT        8080
#define WIFI_AP_SSID            "AthenaOS"

#endif
