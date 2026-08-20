# Board profiles

Each PlatformIO env selects one header via `-DBOARD_*`. Application code includes `boards/board.h` and uses `HAS_*` / pin macros — never hardcode GPIO.

```
pio run -e waveshare_amoled_18
        -> -DBOARD_WAVESHARE_AMOLED_18
        -> firmware/include/boards/board.h
        -> waveshare_amoled_18.h
```

Convention:

- `PIN_NONE` (`-1`) = pin unused; HAL must not `pinMode` it
- `HAS_*=0` = feature absent on that board
- `HAS_*=1` with a stub driver = hardware exists, full driver comes later

## Current envs

| Env | Define | Panel / touch |
|-----|--------|----------------|
| `waveshare_amoled_18` | `BOARD_WAVESHARE_AMOLED_18` | V1 SH8601 + FT3168 |
| `waveshare_amoled_18_v2` | `BOARD_WAVESHARE_AMOLED_18_V2` | V2 CO5300 + CST820 |

Both share [waveshare_amoled_18.h](../firmware/include/boards/waveshare_amoled_18.h).

## Waveshare 1.8 inventory

| Feature | Flag | Notes |
|---------|------|--------|
| Display | `HAS_DISPLAY=1` | 368x448 QSPI CS=12 SCLK=11 D0–D3=4–7 |
| Touch | `HAS_TOUCH=1` | I2C 15/14, INT=21, addr 0x38 or 0x15 |
| Buttons | `BTN1_PIN=0`, `BTN2/3=-1` | BOOT active-low |
| Home hold | `HOME_HOLD_MS=5000` | hold BOOT to return to launcher; `0` = off |
| PWR expander | `HAS_PWR_EXPANDER=1` | TCA9554 @ 0x20 EXIO 4 (mapped as BTN2 short press) |
| IMU | `HAS_IMU=1` | QMI8658 I2C 0x6A/0x6B — real driver |
| Pedometer | `HAS_PEDOMETER=1` | software on IMU — real, step count only |
| Audio | `HAS_AUDIO=1` | ES8311 @ 0x18, I2S BCLK=9 LRC=45 DIN=8 — **stub** |
| Mic | `HAS_MIC=1` | same codec, I2S DOUT=10 — **stub** |
| BLE | `HAS_BLE=1` | on-chip radio — **stub** (`BleController`) |
| Classic BT | `HAS_CLASSIC_BT=0` | off |
| NFC | `HAS_NFC=0` | not onboard; PN532 slot 0x24 IRQ/RST `-1` |
| PMU | `HAS_PMU=1` | AXP2101 @ 0x34 — define only |
| RTC | `HAS_RTC=1` | PCF85063 @ 0x51 — define only |
| SD | `HAS_SD=1` | SDMMC CMD=1 CLK=2 D0=3 — define only |

## Disable a device

In the board header (example: no mic, no third button):

```cpp
#define HAS_MIC       0
#define I2S_DOUT_PIN  PIN_NONE
#define BTN3_PIN      PIN_NONE
```

HAL `begin()` becomes a no-op and main still compiles.

## Add a new board

1. Copy `firmware/include/boards/waveshare_amoled_18.h` to `my_board.h`.
2. Set `BOARD_NAME`, `SCREEN_*`, every `HAS_*` and pin. Use `PIN_NONE` / `0` for missing hardware.
3. Include it from `board.h`:

```cpp
#elif defined(BOARD_MY_BOARD)
#include "my_board.h"
```

4. Add an env in `platformio.ini`:

```ini
[env:my_board]
extends = athena
build_flags =
	${athena.build_flags}
	-DBOARD_MY_BOARD
```

5. Build: `bash ./build_flash.sh --env my_board`

Display/touch drivers in this skeleton are the Waveshare QSPI + I2C pair. A different panel needs a new `display.cpp` branch (or a dedicated driver file) selected by board macros — do not assume SH8601 on every env.
