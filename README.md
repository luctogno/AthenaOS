# AthenaOS

Lightweight app framework for ESP32-S3 devices with a touch display. Dedicated to **Athena Togno** (2026).

Boot: splash → **launcher** → tap **TamaFi** (full pet, same as the TamaFi repo). Hold **BOOT** (`HOME_HOLD_MS`, default 5s) inside an app to return home.

## Hardware (first target)

- **Board:** Waveshare ESP32-S3-Touch-AMOLED-1.8 (368x448, QSPI, Arduino_GFX)
- **Envs:** `waveshare_amoled_18` (V1 SH8601 + FT3168), `waveshare_amoled_18_v2` (V2 CO5300 + CST820)

Pin map and `HAS_*`: [docs/BOARDS.md](docs/BOARDS.md).

## Build & flash (WSL)

Copy `esptool.exe` into the repo root. Then:

```bash
bash ./build_flash.sh
bash ./build_flash.sh --env waveshare_amoled_18_v2 --port COM6
```

[docs/BUILD.md](docs/BUILD.md)

## Layout

```
firmware/src            HAL, splash, app manager, main
apps/launcher           Home screen
apps/tamafi             TamaFi pet
apps/xiaozhi            Xiaozhi wrapper (ESP-IDF stays in submodule)
third_party/xiaozhi-esp32  git submodule 78/xiaozhi-esp32
scripts/build_flash.sh
docs/
```

Xiaozhi submodule: [apps/xiaozhi/README.md](apps/xiaozhi/README.md).
