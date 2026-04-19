#pragma once

// NVS storage keys
#define NVS_NAMESPACE      "countdown"
#define NVS_KEY_SSID       "ssid"
#define NVS_KEY_PASSWORD   "password"
#define NVS_KEY_TARGET     "target_date"
#define NVS_KEY_TZ         "tz"
#define NVS_KEY_CONFIGURED "configured"

// WiFi AP for setup mode
#define AP_SSID "Countdown"

// NTP server
#define NTP_SERVER "pool.ntp.org"

// WiFi connection timeout in milliseconds
#define WIFI_TIMEOUT_MS 15000

// Deep sleep duration in seconds
#define SLEEP_DURATION_SEC 86400   // 24 hours (normal)
#define RETRY_SLEEP_SEC    14400   // 4 hours (sync retry)
#define MAX_RETRIES        6       // max retries before giving up (6 * 4h = 24h)

// NTP sync interval in days
#define SYNC_INTERVAL_DAYS 7

// Reset button (hold on boot to enter setup mode)
#define RESET_BUTTON_PIN   0      // GPIO 0 (Button 1)
#define RESET_HOLD_MS      3000   // Hold 3 seconds to reset

// Battery sense (Heltec Vision Master E213)
#define BATTERY_ADC_PIN    7      // GPIO 7 — ADC1_CH6 tied to VBAT divider
#define BATTERY_CTRL_PIN   46     // GPIO 46 — enables divider when HIGH
#define BATTERY_MULTIPLIER 5.047f // 4.9 * 1.03 — divider + calibration factor
#define BATTERY_FULL_MV    4200   // 4.20 V full
#define BATTERY_EMPTY_MV   3300   // 3.30 V empty (cutoff)

// Charging detection — no VBUS pin on this board, so we infer from voltage.
// A resting full Li-ion sits ~4.15 V; only an active charger holds it at/above 4.20 V.
#define BATTERY_CHARGING_MV      4200
#define CHARGING_POLL_SEC        60     // refresh cadence while charging
#define CHARGING_SYNC_WAKES      1440   // resync NTP once per ~24h while charging
