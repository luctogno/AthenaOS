# Setup

## What this tree is

PlatformIO project for AthenaOS on Waveshare ESP32-S3-Touch-AMOLED-1.8.

- HAL (display, touch, input, IMU, pedometer)
- Splash → launcher → TamaFi as first app
- Board profiles with `HAS_*` / pin `-1`
- Docker build + `esptool.exe` flash script (`--env`)

Not included yet: WiFi setup AP as its own app, Xiazhiu, Home Assistant.

## Environment

1. WSL + Docker, image `tamapetchi-pio:latest` (same as TamaFi)
2. `esptool.exe` in repo root
3. Match the panel revision: V1 → `waveshare_amoled_18`, V2 → `waveshare_amoled_18_v2`

GPIO and I2C addresses: [BOARDS.md](BOARDS.md). Do not use the old idea-doc SPI 240x320 map — that is not this board.

## Flash

```bash
bash ./build_flash.sh
```

Full options: [BUILD.md](BUILD.md).

## After splash works

1. App manager + lifecycle (see [ARCHITECTURE.md](ARCHITECTURE.md))
2. Launcher / system UI
3. WiFi setup AP
4. Fill `apps/` (Xiazhiu, ESPHome/HA, Tamafi)

## Memory (ESP32-S3)

384 KB SRAM + OPI PSRAM on the Waveshare module. This skeleton does not allocate a full framebuffer sprite; keep an eye on heap when the UI layer arrives (`ESP.getFreeHeap()`).
