# cardputer-adv-bubble-level

A round, 2-axis bubble level for the **M5Stack Cardputer-ADV**, using the on-board IMU and display.

## Demo

https://github.com/user-attachments/assets/fb3efb98-453e-4aa0-9718-1e9eb175959b

> Full-quality 1080p source: [`demo/bubble_level.mp4`](demo/bubble_level.mp4).

## Features

- Round bubble-level UI with crosshair and centered target ring.
- Live pitch / roll / total-tilt readout in the corner.
- Bubble turns **green** when you're within 1° of level.
- **Zero / calibrate**: press **C** or **space** to capture the current pose as "level" (compensates for the table you set the device on).
- **Reset**: press **R** to go back to true gravity.

## Hardware

- M5Stack **Cardputer-ADV** (ESP32-S3 + IMU + 240×135 display + QWERTY keyboard).

## Build & flash (Arduino IDE)

1. Install the **esp32** boards package (and optionally the **M5Stack** board manager package for the preset Cardputer entry).
2. In **Library Manager**, install:
   - `M5Cardputer` (pulls in `M5Unified` and `M5GFX` as dependencies)
3. Open `bubble_level.ino`.
4. **Tools → Board**: `M5Stack-CardputerADV` (or, on the plain espressif package, `ESP32S3 Dev Module` with: USB CDC On Boot = **Enabled**, Flash Size = **16MB**, PSRAM = **OPI PSRAM**).
5. Pick the port, click **Upload**.

## Controls

| Key       | Action                                                    |
| --------- | --------------------------------------------------------- |
| **C** / space | Capture current tilt as the new zero (the bubble centers) |
| **R**     | Clear calibration; show true gravity again                |

## Tweaking

All knobs live at the top of `bubble_level.ino`:

- `SMOOTHING` — IIR low-pass coefficient. Higher = snappier; lower = calmer.
- `FULL_SCALE_G` — Acceleration that pushes the bubble fully to the rim. Lower it (e.g. `0.25f`) for a more sensitive level.
- `LEVEL_TOL_DEG` — Tilt threshold under which the bubble turns green.

If the bubble runs the wrong way on either axis, flip the sign of `fx` or `fy` where `bx` / `by` are computed inside `loop()`.
