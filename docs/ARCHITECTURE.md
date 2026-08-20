# Architecture

```
┌─────────────────────────────────┐
│  Apps                           │
│  launcher / tamafi / xiaozhi    │
├─────────────────────────────────┤
│  App manager                    │
├─────────────────────────────────┤
│  Splash (boot only)             │
├─────────────────────────────────┤
│  HAL                            │
│  Display Touch Input Imu        │
│  Pedometer Audio Mic Nfc Ble    │
├─────────────────────────────────┤
│  boards/*.h  (compile-time)     │
└─────────────────────────────────┘
```

## Boot

1. HAL probe (display, touch, IMU, …)
2. Splash ~1.8s (owl mark + progress)
3. `launcher` starts immediately
4. Tap **TamaFi** to run the pet (TamaFi game loop, WiFi, pedometer feed)
5. Hold **BOOT** (`HOME_HOLD_MS`, default 5000) → launcher

TamaFi is a port of the working TamaFi sources under `apps/tamafi/`, using AthenaOS Display/Touch/Imu. It polls its own input (`consumesInput()`), so the launcher does not steal touches.

## App API

Apps inherit `App` (`init/start/update/draw/pause/resume/stop`) and register from `main.cpp` via `registerLauncherApp()` / `registerTamafiApp()` / `registerXiaozhiApp()`.

Launcher icons: set `AppManifest.icon` (`APP_ICON_PET`, `APP_ICON_MIC`, `APP_ICON_DEFAULT`). For a custom RGB565 bitmap, set `iconBitmap` / `iconW` / `iconH` (drawn instead of the glyph).

Xiaozhi upstream lives in `third_party/xiaozhi-esp32` (git submodule, ESP-IDF). AthenaOS only compiles `apps/xiaozhi/` — see [apps/xiaozhi/README.md](../apps/xiaozhi/README.md).

## Roadmap

- WiFi setup AP as its own app
- Xiazhiu, Home Assistant
- LittleFS for OS settings (TamaFi already uses Preferences)

