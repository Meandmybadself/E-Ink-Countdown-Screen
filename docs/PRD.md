# Countdown to 2028 - Heltec Vision Master E213

A countdown application for the **Heltec Vision Master E213 ESP32-S3R8 LoRa Development Board** that displays the number of days remaining until January 1, 2028 on the 2.13" e-ink display.

## Features

- **Real-time countdown** calculation based on NTP synchronized time
- **E-ink display** for low power consumption
- **WiFi connectivity** for automatic time synchronization
- **Deep sleep mode** - updates once per hour to maximize battery life
- **Large, readable display** showing days remaining in an easy-to-read format

## Hardware Requirements

- **Heltec Vision Master E213** (ESP32-S3R8)
  - ESP32-S3 microcontroller with 8MB PSRAM
  - 2.13" e-ink display (250x122 pixels, black & white)
  - WiFi/BLE connectivity
  - USB-C cable for programming and power


## Configuration

Before uploading, edit [src/config.h](src/config.h) with your settings:

### WiFi Settings

```cpp
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### Timezone Settings

Adjust `GMT_OFFSET_SEC` for your timezone:

```cpp
const long GMT_OFFSET_SEC = 0;  // UTC
// Examples:
// -28800 for PST (UTC-8)
// -18000 for EST (UTC-5)
// 3600 for CET (UTC+1)
// 28800 for CST China (UTC+8)
```

### Target Date

The default target is January 1, 2028. To change it:

```cpp
const int TARGET_YEAR = 2028;
const int TARGET_MONTH = 1;
const int TARGET_DAY = 1;
```

### Update Interval

Change how often the display updates (in seconds):

```cpp
const int SLEEP_DURATION = 3600;  // 3600 = 1 hour
// For testing, use 60 seconds
```

## How It Works

1. **Boot**: Device wakes from deep sleep (or initial power-on)
2. **WiFi Connection**: Connects to configured WiFi network
3. **Time Sync**: Synchronizes with NTP server to get current date/time
4. **Calculation**: Calculates days remaining until target date
5. **Display Update**: Updates e-ink display with countdown
6. **Sleep**: Enters deep sleep mode for the configured duration
7. **Repeat**: Wakes up after sleep timer expires

## Power Consumption

The e-ink display and deep sleep mode make this project extremely power-efficient:

- **Active time**: ~10-15 seconds per update (WiFi + display refresh)
- **Sleep current**: ~10-20 µA
- **With hourly updates**: Battery can last several months on a small LiPo battery

## Troubleshooting

### Display not working

- Ensure WiFi credentials are correct
- Check that the board is properly powered via USB-C
- Verify the heltec-eink-modules library is installed

### WiFi connection fails

- Check SSID and password in [src/config.h](src/config.h)
- Ensure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Move closer to WiFi router

### Time not syncing

- Check internet connectivity
- Try a different NTP server (e.g., "time.google.com", "time.nist.gov")
- Verify firewall isn't blocking NTP (port 123)

### Display shows incorrect countdown

- Verify timezone settings (`GMT_OFFSET_SEC`)
- Check that NTP sync was successful (view Serial Monitor)
- Ensure target date is configured correctly

## Serial Monitor Output

For debugging, connect to the Serial Monitor at 115200 baud to see:

```
Boot count: 1
Connecting to WiFi...
WiFi connected!
IP address: 192.168.1.100
Syncing time with NTP server...
Time synchronized!
Current time: 2025-12-20 14:30:00
Days until 2028-01-01: 743
Display updated!
Entering deep sleep for 3600 seconds...
```

## Customization Ideas

- Change target date to countdown to birthdays, holidays, or events
- Modify display layout for different text sizes and positioning
- Add battery voltage monitoring
- Display additional information (current date, time, etc.)
- Adjust sleep duration based on how close to target date