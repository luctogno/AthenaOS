# Xiaozhi as an AthenaOS app (git submodule)

Upstream [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) is **ESP-IDF**, not Arduino/PlatformIO. `app_main()` initializes NVS and then `Application::Run()` **never returns**. Compiling that tree into the AthenaOS PIO firmware is not possible without forking it.

Layout:

```
third_party/xiaozhi-esp32/   git submodule (do not edit)
apps/xiaozhi/                AthenaOS wrapper only
```

## One-time: submodule

AthenaOS must be a git repo. From the AthenaOS root (PowerShell):

```powershell
git init
git submodule add https://github.com/78/xiaozhi-esp32.git third_party/xiaozhi-esp32
git submodule update --init --recursive
```

If `xiaozhi-esp32` was already cloned at the repo root, move it then register the gitlink (no re-clone):

```powershell
New-Item -ItemType Directory -Force third_party | Out-Null
Move-Item -Force xiaozhi-esp32 third_party\xiaozhi-esp32
```

Then add `.gitmodules` (already in this repo):

```ini
[submodule "third_party/xiaozhi-esp32"]
	path = third_party/xiaozhi-esp32
	url = https://github.com/78/xiaozhi-esp32.git
```

On a fresh clone of AthenaOS:

```bash
git submodule update --init --recursive
```

Update upstream later:

```bash
git -C third_party/xiaozhi-esp32 fetch
git -C third_party/xiaozhi-esp32 checkout main
git -C third_party/xiaozhi-esp32 pull
```

Never change files under `third_party/xiaozhi-esp32`. AthenaOS code goes in `apps/xiaozhi/`.

## Wrapper

`apps/xiaozhi/xiaozhi_app.cpp` registers as launcher app `xiaozhi`. It uses AthenaOS Display/Input only. It does **not** `#include` IDF headers from the submodule.

Official IDF board for this hardware:

- V1: `waveshare-esp32-s3-touch-amoled-1.8`
- V2: `waveshare-esp32-s3-touch-amoled-1.8-v2`

To run the **full** Xiaozhi voice stack (wake word, ASR, TTS), build that board with ESP-IDF from the submodule, as a separate firmware — not via `build_flash.sh`.

## Next (when you want the real assistant)

Port incrementally behind the wrapper: audio HAL (`HAS_AUDIO` / `HAS_MIC`) first, then protocol. Still keep upstream unmodified; new glue stays in `apps/xiaozhi/`.
