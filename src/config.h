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
