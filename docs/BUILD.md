# Build and flash

Same flow as TamaFi: Docker image `tamapetchi-pio:latest`, rsync to `~/AthenaOS` when the repo is on `/mnt/*`, PlatformIO cache volume `tamapetchi-pio-cache`, flash with `esptool.exe` from the repo root.

## Prerequisites

- WSL + Docker
- Image `tamapetchi-pio:latest` already used for TamaFi / TamaPetchi
- `esptool.exe` copied to the **repo root** (gitignored)

## Commands (WSL, repo root)

```bash
# default env: waveshare_amoled_18
bash ./build_flash.sh

# V2 panel
bash ./build_flash.sh --env waveshare_amoled_18_v2 --port COM6

# list envs from platformio.ini
bash ./build_flash.sh --list-envs

# pick COM port interactively
bash ./build_flash.sh -i

# build only
bash ./build_flash.sh --no-flash

# flash bins already in .pio-out/<env>/
bash ./build_flash.sh --flash-only --env waveshare_amoled_18
```

Wrapper `build_flash.sh` in the repo root forwards to `scripts/build_flash.sh`.

## Outputs

Firmware bins land in `.pio-out/<env>/` (`bootloader.bin`, `partitions.bin`, `firmware.bin`) so V1 and V2 do not overwrite each other.

Flash offsets (ESP32-S3 Arduino): `0x0` bootloader, `0x8000` partitions, `0x10000` firmware. Override chip with `--chip` if a future board is not `esp32s3`.

## Serial

```bash
pio device monitor -b 115200
```

Expected boot:

```
=== AthenaOS Booting ===
=== AthenaOS probe ===
board=Waveshare AMOLED 1.8 ...
display=1 touch=1 ...
AthenaOS ready
```

If flash fails, hold BOOT, tap RESET, release BOOT, retry. The script offers a retry prompt.
