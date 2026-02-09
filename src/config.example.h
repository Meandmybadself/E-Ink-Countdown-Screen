#pragma once

// WiFi Settings
const char* WIFI_SSID     = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";

// Timezone: offset from UTC in seconds
// Examples:
// -28800 for PST (UTC-8)
// -18000 for EST (UTC-5)
//  3600  for CET (UTC+1)
//  28800 for CST China (UTC+8)
const long GMT_OFFSET_SEC     = 0;
const int  DAYLIGHT_OFFSET_SEC = 0;

// Target date for countdown
const int TARGET_YEAR  = 2028;
const int TARGET_MONTH = 1;
const int TARGET_DAY   = 1;

// Deep sleep duration in seconds (3600 = 1 hour)
const int SLEEP_DURATION = 3600;

// NTP server
const char* NTP_SERVER = "pool.ntp.org";

// WiFi connection timeout in milliseconds
const int WIFI_TIMEOUT_MS = 15000;
