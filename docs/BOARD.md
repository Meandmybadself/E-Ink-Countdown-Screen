# Heltec Vision Master E213 — Board Reference

## Hardware Overview

- **MCU:** ESP32-S3R8 (dual-core Xtensa LX7, 240MHz)
- **Flash:** 8MB
- **PSRAM:** 8MB (QSPI)
- **Display:** 2.13" E-Ink, 250x122 pixels, black & white
- **Radio:** SX1262 LoRa
- **USB:** USB-Serial/JTAG (CDC mode)
- **Board Revision:** V1.1 (uses `EInkDisplay_VisionMasterE213V1_1` class)

## Pin Map

### E-Ink Display (internally wired, no configuration needed)

| Function | GPIO |
|----------|------|
| BUSY     | 1    |
| DC       | 2    |
| RST      | 3    |
| CLK      | 4    |
| CS       | 5    |
| SDI/MOSI | 6    |

### LoRa Radio (SX1262)

| Function | GPIO |
|----------|------|
| NSS      | 8    |
| SCK      | 9    |
| MOSI     | 10   |
| MISO     | 11   |
| NRST     | 12   |
| BUSY     | 13   |
| DIO1     | 14   |

### General I/O

| Function   | GPIO |
|------------|------|
| I2C SDA    | 39   |
| I2C SCL    | 38   |
| UART TX    | 43   |
| UART RX    | 44   |
| Vext Power | 18   |
| Button 1   | 0    |
| Button 2   | 21   |

### Power Control

- **Vext (GPIO 18):** Active HIGH. Powers the display and I2C connector. Managed automatically by the display library.
- Manual control: `Platform::VExtOn()` / `Platform::VExtOff()`

## Build Environment

### PlatformIO Configuration

```ini
[env:vision-master-e213]
platform = espressif32
board = heltec_wifi_lora_32_V3
framework = arduino
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
board_upload.use_1200bps_touch = true
lib_extra_dirs = .
build_flags =
  -D ARDUINO_USB_CDC_ON_BOOT=1
  -D Vision_Master_E213
```

Both `-D ARDUINO_USB_CDC_ON_BOOT=1` and `-D Vision_Master_E213` are required. The first enables serial output over USB, the second tells the eink library which pin configuration to use.

### Common Commands

```bash
pio run                  # Build
pio run -t upload        # Build and flash
pio device monitor       # Serial monitor (115200 baud)
pio run -t upload && pio device monitor  # Flash then monitor
```

## Display Library — heltec-eink-modules v4.6.0

Third-party library by Todd Herbert. Uses the Adafruit GFX API. Located in `./heltec-eink-modules/`.

No external dependencies — GFX and SdFat are bundled.

### Initialization

```cpp
#include <heltec-eink-modules.h>

EInkDisplay_VisionMasterE213V1_1 display;
```

No `begin()` call needed. No pin parameters needed. The display is ready to use immediately.

### Orientation

```cpp
display.landscape();            // 250 wide x 122 tall (USB port on left)
display.portrait();             // 122 wide x 250 tall (USB port below)
display.setRotation(0);         // 0, 90, 180, 270
display.setRotation(USB_ABOVE); // USB_ABOVE, USB_LEFT, USB_BELOW, USB_RIGHT
```

### Drawing Text

```cpp
display.setCursor(x, y);
display.print("text");
display.println("text with newline");
display.printCenter("centered text");
display.printCenter("offset text", x_offset, y_offset);

display.setTextColor(BLACK);           // BLACK or WHITE
display.setTextSize(2);                // Integer scale factor
display.setFont(&FreeSans9pt7b);       // Custom font
display.setFont(NULL);                 // Default 5x7 font
```

Text measurement:

```cpp
display.getTextWidth("text");
display.getTextHeight("text");
display.getTextCenterX("text");
display.getTextCenterY("text");
```

### Drawing Shapes (Adafruit GFX)

```cpp
display.drawPixel(x, y, color);
display.drawLine(x0, y0, x1, y1, color);
display.drawRect(x, y, w, h, color);
display.fillRect(x, y, w, h, color);
display.drawCircle(x, y, r, color);
display.fillCircle(x, y, r, color);
display.drawTriangle(x0, y0, x1, y1, x2, y2, color);
display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
display.drawXBitmap(x, y, bitmap, w, h, color);
```

Colors: `BLACK`, `WHITE`

### Display Control

```cpp
display.update();          // Push buffer to display (full refresh, ~4s)
display.clear();           // Clear display and buffer to background color
display.clearMemory();     // Clear buffer only (display unchanged)
display.setBackgroundColor(BLACK);  // Default is WHITE
```

### Screen Dimensions

```cpp
display.width();    // Current width (depends on rotation)
display.height();   // Current height (depends on rotation)
display.centerX();  // Horizontal center
display.centerY();  // Vertical center
```

### Fast Mode (Partial Refresh)

Faster updates (~1s) but lower quality with ghosting. Use sparingly.

```cpp
display.clear();          // Always do a full refresh first
display.fastmodeOn();
display.print("fast 1");
display.update();         // ~1s partial refresh
display.print("fast 2");
display.update();         // ~1s partial refresh
display.fastmodeOff();
display.update();         // Full refresh to clean up artifacts
```

### Windowed Updates

Draw to a sub-region of the screen. Window width must be a multiple of 8.

```cpp
display.setWindow(left, top, width, height);
display.print("in window");
display.update();         // Only the window region refreshes
display.fullscreen();     // Return to full screen
```

### Fonts

Include from `Fonts/` directory:

```cpp
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold18pt7b.h"
```

Available families: **FreeMono**, **FreeSans**, **FreeSerif** — each in 9pt, 12pt, 18pt, 24pt with Regular, Bold, Oblique/Italic, and BoldOblique/BoldItalic variants.

Tiny fonts: `Picopixel`, `TomThumb`, `Org_01`, `Tiny3x3a2pt7b`

### SD Card (Optional)

```cpp
display.useSD(40, 41);                        // CS pin, MISO pin
display.loadFullscreenBMP("image.bmp");       // Load full-screen BMP
display.drawMonoBMP(x, y, "icon.bmp", BLACK); // Draw partial BMP
display.saveToSD("screenshot.bmp");           // Save screen to SD
display.update();
```

Suggested wiring: SD CS on GPIO 40, SD MISO on GPIO 41, MOSI/SCK shared with display (GPIO 6/4).

### Images (XBitmap)

```cpp
display.drawXBitmap(x, y, bitmap_array, width, height, BLACK);
```

### Invert Colors

```cpp
display.invert(left, top, width, height);
display.update();
```

## Minimal Working Example

```cpp
#include <heltec-eink-modules.h>

EInkDisplay_VisionMasterE213V1_1 display;

void setup() {
    display.landscape();
    display.printCenter("Hello, World!");
    display.update();
}

void loop() {
}
```

## Flash Partition Layout (8MB)

| Partition | Type     | Offset     | Size   |
|-----------|----------|------------|--------|
| nvs       | data/nvs | 0x9000     | 20KB   |
| otadata   | data/ota | 0xe000     | 8KB    |
| app0      | app/ota0 | 0x10000    | 3.2MB  |
| app1      | app/ota1 | 0x340000   | 3.2MB  |
| spiffs    | data     | 0x670000   | 1.5MB  |
| coredump  | data     | 0x7F0000   | 64KB   |

## Troubleshooting

- **Display blank:** Confirm you are using `EInkDisplay_VisionMasterE213V1_1` (not the non-V1.1 variant).
- **No serial output:** Ensure `-D ARDUINO_USB_CDC_ON_BOOT=1` is in build_flags.
- **Upload fails:** The board uses 1200bps touch reset (`board_upload.use_1200bps_touch = true`). If upload stalls, hold Button 1 (GPIO 0) while pressing reset, then retry.
- **Ghosting on display:** Do a full `display.clear()` before and after using fast mode.
- **Window width errors:** Window width must be a multiple of 8 (hardware limitation).
