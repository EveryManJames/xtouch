# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`xtouch` is ESP32 firmware that turns a cheap TFT touchscreen board into a companion
control/monitoring panel for BambuLab 3D printers. It talks to the printer over local
MQTT (TLS), renders an LVGL UI, and is configured from a JSON file on an SD card. The
only tested target is the 2.8" ESP32-2432S028R board (aka "cheap yellow display").

## Build / flash / monitor

This is a **PlatformIO** project (`platformio.ini`), not CMake/Make. Single env: `esp32dev`.

```bash
pio run                      # compile firmware
pio run -t upload            # build + flash over USB
pio device monitor           # serial monitor @ 115200 (esp32_exception_decoder filter)
pio run -t clean             # clean build artifacts (.pio/)
```

There are no unit tests — this is embedded firmware validated on-device via the serial
monitor. `src/xtouch/debug.h` defines `ConsoleLog/ConsoleInfo/ConsoleDebug/ConsoleError`
macros (compile-time gated) for logging over serial.

### Required one-time library configuration

Two vendored config headers in `resources/` must be copied into the PlatformIO library
folders before/after `lib_deps` are fetched, following the standard TFT_eSPI + LVGL setup:

- `resources/User_Setup.h` → the `TFT_eSPI` library (ILI9341 driver + pin map for this board)
- `resources/lv_conf.h` → next to the `lvgl` library folder

Without these, the display driver and LVGL will not match the hardware.

## Versioning & build scripts (important, non-obvious)

`scripts/` hooks are wired into the build via `platformio.ini`:

- **`version.py`** (`build_flags` `!python3`): reads `version.json` and injects
  `-D XTOUCH_FIRMWARE_VERSION="x.y.z"` at compile time.
- **`pre-build.py`** → runs `scripts/download-errors.js`, which **fetches BambuLab's live
  HMS/error-code table** from `e.bambulab.com` and regenerates `src/xtouch/bbl/bbl-errors.c`
  and `.h` (PROGMEM string tables). A network connection is needed for a clean build, and
  these two generated files will change — that is expected, not a manual edit.
- **`post-build.py`** → auto-increments the patch in `version.json`, then copies/merges the
  firmware into a **sibling `../xtouch-bin/` repo** (OTA `.bin`, `ota.json`, WebUSB merged
  binary + manifest) using `esptool.py`. This assumes `../xtouch-bin` exists relative to the
  repo root; it will fail otherwise. Expect `version.json` to bump on every successful build.

## Architecture

### Header-only / unity-include style

Almost the entire firmware lives in **`.h` files under `src/xtouch/`** that contain full
function definitions (not just declarations). `src/main.cpp` `#include`s them in a specific
order, and each subsystem `.h` includes the ones it depends on. There is essentially one
translation unit. Consequences to respect:

- **Include order in `main.cpp` is load-bearing** — a subsystem's globals must be defined
  before code that uses them. Adding a new subsystem usually means a new `.h` and an
  `#include` placed at the right point in `main.cpp`.
- Global state lives in structs defined in `src/xtouch/types.h` (`bambuStatus`,
  `xTouchConfig`, `controlMode`) and is referenced directly across files.
- The `.c`/`.cpp` files that do exist are mostly the LVGL/SquareLine UI and a few helpers
  (`ams.c`, `errors.c`, `autogrowstream.cpp`, `bbl/bbl-errors.c`).

### `setup()` boot sequence (`src/main.cpp`)

EEPROM → globals → screen → intro UI → **block until SD card** → coldboot delay →
load settings → check SD-card firmware update → touch calibration → **block until WiFi** →
check online OTA → screen timers → global events → MQTT setup → chamber sensor timer.
Both SD and WiFi setup are `while(!...)` spin loops — the device will not proceed without them.

### LVGL message bus decouples UI from printer logic

The UI layer and the printer/MQTT layer do **not** call each other directly. They communicate
through LVGL's `lv_msg` pub/sub, with all message IDs enumerated in `src/ui/ui_msgs.h`
(`XTOUCH_ON_*` = inbound printer state, `XTOUCH_COMMAND_*` = outbound UI actions,
`XTOUCH_SETTINGS_*`, `XTOUCH_FIRMWARE_*`).

- Incoming MQTT status → `mqtt.h` parses JSON into `bambuStatus` → `lv_msg_send(XTOUCH_ON_...)`
  → UI screens subscribed to that ID update their widgets.
- UI button → sends `XTOUCH_COMMAND_...` → `events.h` / device layer handles it → publishes
  an MQTT command to the printer.
- `src/xtouch/events.h` wires settings-related subscriptions in `xtouch_setupGlobalEvents()`.

When adding a feature, the pattern is: add a message ID in `ui_msgs.h`, publish it from one
side, subscribe on the other.

### Key subsystems (`src/xtouch/`)

- `mqtt.h` — the core: TLS connection to the printer, parses the large push-status JSON into
  `bambuStatus`, fans out `lv_msg` updates. Largest/most important file.
- `device.h` — builds and publishes BambuLab MQTT command payloads (print control, moves,
  temps, fans, filament); manages the sequence-id counter.
- `bblp.h`, `ams.h`/`ams.c`, `hms.h` — BambuLab protocol helpers, AMS (filament system) state,
  and health-management-system error decoding (uses the generated `bbl/bbl-errors.*` tables).
- `config.h` / `settings.h` — `config.h` reads immutable provisioning from `/xtouch.json` on SD
  (WiFi creds, printer host/serial/access code); `settings.h` reads/writes user-adjustable
  settings in `/xtouch/settings.json`. Do not confuse the two files. SD paths are centralized
  in `paths.h`.
- `net.h` / `firmware.h` — HTTPS downloads (with MD5 verify) and OTA / SD-card firmware update
  and semver comparison.
- `sensors/chamber.h` — optional DS18B20 chamber temperature sensor (OneWire).
- `filesystem.h`, `sdcard.h`, `eeprom.h` — SD/LittleFS JSON read/write, touch-calibration
  persistence.
- `bbl-certs.h` — pinned CA certs for the printer MQTT and update servers.

### UI layer (`src/ui/`)

Generated with **SquareLine Studio** (LVGL 8.3). Treat it as generated code:
`screens/` (one file per screen: home, temperature, control, filament, settings, intro),
`components/`, `fonts/`, plus `ui.c/h`, `ui_events.*`, `ui_helpers.*`, `ui_loaders.*`.
Prefer editing behavior through the message bus and event handlers rather than hand-rewriting
generated widget layout, since regenerating from SquareLine would overwrite it.

### Hardware abstraction (`src/devices/`)

`src/devices/2.8/` (`screen.h`, `touch.h`) holds pin maps and driver glue for the 2.8" board,
selected by the `-D__XTOUCH_SCREEN_28__` build flag. New boards would add a directory here
guarded by a new screen-size define.

## `xtouch28/` — unrelated to the firmware

A Chrome MV3 extension ("xtouch lite provisioning portal"), a separate deliverable from the
ESP32 firmware. Provisioning now happens via the web installer + SD-card `xtouch.json`
(see `Readme.md`), so most firmware work does not touch this directory.
