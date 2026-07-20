# Building & flashing the 5" (Guition JC8048W550) firmware

This is the ESP32-S3 / 800×480 target. It builds from the same repo as the
2.8" board but uses a separate PlatformIO environment: **`jc8048w550`**. The
2.8" build (`esp32dev`) is unaffected.

## Prerequisites

- **PlatformIO** — either the *PlatformIO IDE* VS Code extension (easiest) or
  PlatformIO Core (`pip install platformio`).
- **Node.js** on your PATH — the pre-build step runs
  `node scripts/download-errors.js` to regenerate the BambuLab HMS/error tables.
  No Node = the build fails before compiling. Check with `node --version`.
- **Internet** for the first build: it downloads the ESP32-S3 toolchain and all
  libraries (a few hundred MB, several minutes, one time).

## 1. Get the code

```bash
git checkout claude/init-sq7wjp
```

## 2. First build

### VS Code (PlatformIO IDE)
1. Install VS Code + the **PlatformIO IDE** extension.
2. **File → Open Folder →** this repo. Let the PlatformIO status bar finish init.
3. PlatformIO ant-head icon → **Project Tasks → `jc8048w550` → General → Build**
   (or set the env in the bottom bar and press the ✓).

### CLI
```bash
pio run -e jc8048w550
```

The first run downloads `espressif32 @ 6.5.0` (pinned — arduino-esp32 2.x, which
the RGB panel driver requires) plus the libraries, then compiles `src/`.

## 3. One-time library config (required)

LVGL needs its config header copied into the fetched libraries. Do it **after**
the first build has downloaded `lib_deps`, then build again:

```bash
cp resources/lv_conf.h .pio/libdeps/jc8048w550/   # next to the lvgl/ folder
pio run -e jc8048w550
```

Copy the **current** `resources/lv_conf.h` — it enables the Montserrat
16/20/24/36 fonts the 5" UI uses.

> The 5" target uses **Arduino_GFX**, not TFT_eSPI, so `resources/User_Setup.h`
> is **not** needed here (that's only for the 2.8" build).

## 4. Prepare the SD card (required to boot past setup)

Boot blocks on `while(!SD)` then `while(!WiFi)`. On a **FAT32** card, put
`xtouch.json` at the root:

```json
{
  "ssid": "your-wifi",
  "pwd": "your-password",
  "timeout": "3000",
  "coldboot": "5000",
  "mqtt": {
    "host": "192.168.x.x",
    "accessCode": "12345678",
    "serialNumber": "01S00...",
    "printerModel": "P1S"
  }
}
```

Insert it into the board's TF slot before powering on.

## 5. Flash & monitor

```bash
pio run -e jc8048w550 -t upload      # build + flash over USB-C
pio device monitor -e jc8048w550     # serial @ 115200
```

If the upload can't grab the port, put the S3 in download mode: hold **BOOT**,
tap **RESET**, release **BOOT**, then re-run upload.

## Hardware notes / first-boot checklist

The 5" support was written against the documented board config and reviewed
statically. Confirm on hardware:

- **Display blank/garbled** → RGB panel pins/timing in
  `src/devices/5.0/screen.h`.
- **Touch unresponsive** → GT911 address/reset in `src/devices/5.0/touch.h`
  (try `GT911_ADDR2` / an INT/RST pin for your board revision).
- **Serial monitor silent** → some S3 boards use native USB CDC; if so, add
  `-DARDUINO_USB_CDC_ON_BOOT=1` and `-DARDUINO_USB_MODE=1` to the `jc8048w550`
  `build_flags`.
- **LED strip** (optional, Accessories → Lighting): data on **GPIO 18** (or 17;
  both are on the Extended IO headers), power more than a few LEDs from an
  external 5V supply with a common ground; a 3.3V→5V level shifter is advised
  for long runs.

## Notes

- The `post-build.py` OTA/WebUSB packaging (and the sibling `../xtouch-bin`
  repo) only runs for the `esp32dev` env; the `jc8048w550` build just
  increments `version.json`.
- Pin/theme/LED reference: `src/devices/5.0/`, `src/ui5/`, `src/xtouch/led.h`.
