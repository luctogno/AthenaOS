# Quick start

## 1. Copy esptool

Place `esptool.exe` in the AthenaOS repo root (same file you use for TamaFi).

## 2. Build and flash (WSL)

```bash
cd /mnt/c/Users/Luca/Personale/AthenaOS
bash ./build_flash.sh --port COM6
```

V2 panel: `--env waveshare_amoled_18_v2`.

## 3. Serial

```bash
pio device monitor -b 115200
```

You should see a splash (owl + AthenaOS), then the **launcher**. Tap **TamaFi** for the pet. Hold BOOT (`HOME_HOLD_MS`, 5s) to return home.

## 4. Next

See [ARCHITECTURE.md](ARCHITECTURE.md) for the app framework still to build, and [BOARDS.md](BOARDS.md) to add or disable peripherals.
