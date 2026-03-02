#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "storage.h"
#include "portal.h"
#include "display.h"

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int daysRemaining = -1;   // -1 = needs initial sync
RTC_DATA_ATTR uint8_t daysSinceSync = 0;
RTC_DATA_ATTR bool isRetrying = false;
RTC_DATA_ATTR uint8_t retryCount = 0;

static bool inSetupMode = false;

// --- WiFi & Time helpers ---

static bool connectWiFi(const char* ssid, const char* password) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_TIMEOUT_MS) {
            Serial.println(" FAILED!");
            return false;
        }
        delay(250);
        Serial.print(".");
    }

    Serial.println(" connected!");
    return true;
}

static bool syncTime(const char* tz) {
    Serial.println("Syncing time with NTP...");
    configTzTime(tz, NTP_SERVER);

    struct tm timeinfo;
    for (int i = 0; i < 10; i++) {
        if (getLocalTime(&timeinfo, 500)) {
            char buf[32];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
            Serial.printf("Time synced: %s\n", buf);
            return true;
        }
    }
    Serial.println("NTP sync FAILED!");
    return false;
}

// --- Date calculation ---

static long dateToDays(int y, int m, int d) {
    if (m <= 2) { y--; m += 12; }
    return 365L * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + d;
}

static int calculateDaysRemaining(int targetYear, int targetMonth, int targetDay) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return -1;

    int nowY = timeinfo.tm_year + 1900;
    int nowM = timeinfo.tm_mon + 1;
    int nowD = timeinfo.tm_mday;

    long today  = dateToDays(nowY, nowM, nowD);
    long target = dateToDays(targetYear, targetMonth, targetDay);
    long diff   = target - today;

    Serial.printf("Today: %04d-%02d-%02d, Target: %04d-%02d-%02d, Diff: %ld\n",
                  nowY, nowM, nowD, targetYear, targetMonth, targetDay, diff);

    return (diff > 0) ? (int)diff : 0;
}

// --- Sleep ---

static void deepSleep(int seconds) {
    Serial.printf("Sleeping %d seconds...\n", seconds);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}

// --- Sync attempt ---

static bool trySync(const char* ssid, const char* password, const char* tz) {
    if (!connectWiFi(ssid, password)) return false;
    bool ok = syncTime(tz);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return ok;
}

// --- Main ---

void setup() {
    Serial.begin(115200);
    delay(100);

    bootCount++;
    Serial.printf("\nBoot #%d | days=%d | sync=%d | retry=%d/%d\n",
                  bootCount, daysRemaining, daysSinceSync, isRetrying, retryCount);

    // Check if device is configured
    if (!storageIsConfigured()) {
        Serial.println("No config — setup mode");
        inSetupMode = true;
        displayShowSetupMode();
        portalStart();
        return;
    }

    // Load config
    String ssid, password, targetDate, tz;
    if (!storageLoadConfig(ssid, password, targetDate, tz)) {
        Serial.println("Config read error — setup mode");
        storageClear();
        inSetupMode = true;
        displayShowSetupMode();
        portalStart();
        return;
    }

    int targetYear, targetMonth, targetDay;
    sscanf(targetDate.c_str(), "%d-%d-%d", &targetYear, &targetMonth, &targetDay);

    // After power loss, RTC is cleared — try NVS backup
    if (daysRemaining < 0) {
        daysRemaining = storageLoadDays();
        Serial.printf("Loaded days from NVS: %d\n", daysRemaining);
    }

    // --- Retry wake: just try to sync, don't decrement ---
    if (isRetrying) {
        Serial.println("Retry wake — attempting sync");
        if (trySync(ssid.c_str(), password.c_str(), tz.c_str())) {
            // Sync succeeded — recalculate from real time
            int days = calculateDaysRemaining(targetYear, targetMonth, targetDay);
            if (days >= 0) daysRemaining = days;
            isRetrying = false;
            retryCount = 0;
            daysSinceSync = 1;
            storageSaveDays(daysRemaining);
            displayShowCountdown(daysRemaining, targetYear, targetMonth, targetDay, false);
            Serial.println("Sync recovered!");
        } else {
            retryCount++;
            Serial.printf("Retry %d/%d failed\n", retryCount, MAX_RETRIES);
            if (retryCount >= MAX_RETRIES) {
                // Give up retrying, resume daily schedule
                isRetrying = false;
                retryCount = 0;
                daysSinceSync++;
                Serial.println("Max retries — resuming daily mode");
                deepSleep(SLEEP_DURATION_SEC);
                return;
            }
            // Don't touch display, sleep 4h and try again
            deepSleep(RETRY_SLEEP_SEC);
            return;
        }
        deepSleep(SLEEP_DURATION_SEC);
        return;
    }

    // --- First boot ever: need initial sync to establish baseline ---
    if (daysRemaining < 0) {
        Serial.println("No baseline — must sync");
        if (trySync(ssid.c_str(), password.c_str(), tz.c_str())) {
            daysRemaining = calculateDaysRemaining(targetYear, targetMonth, targetDay);
            if (daysRemaining < 0) daysRemaining = 0;
            daysSinceSync = 1;
            storageSaveDays(daysRemaining);
            displayShowCountdown(daysRemaining, targetYear, targetMonth, targetDay, false);
            deepSleep(SLEEP_DURATION_SEC);
        } else {
            // Can't establish baseline — retry in 4h
            // Don't touch the display, just sleep and retry
            Serial.println("Initial sync failed — retry in 4h");
            isRetrying = true;
            retryCount = 1;
            deepSleep(RETRY_SLEEP_SEC);
        }
        return;
    }

    // --- Normal daily wake ---
    daysRemaining--;
    if (daysRemaining < 0) daysRemaining = 0;
    daysSinceSync++;
    Serial.printf("Daily update: %d days remaining\n", daysRemaining);

    bool needsSync = (daysSinceSync >= SYNC_INTERVAL_DAYS);
    bool syncFailed = false;

    if (needsSync) {
        Serial.println("Weekly sync due");
        if (trySync(ssid.c_str(), password.c_str(), tz.c_str())) {
            int days = calculateDaysRemaining(targetYear, targetMonth, targetDay);
            if (days >= 0) daysRemaining = days;
            daysSinceSync = 1;
            Serial.printf("Sync corrected to %d days\n", daysRemaining);
        } else {
            syncFailed = true;
            isRetrying = true;
            retryCount = 0;
            Serial.println("Sync failed — entering retry mode");
        }
    }

    storageSaveDays(daysRemaining);
    displayShowCountdown(daysRemaining, targetYear, targetMonth, targetDay, syncFailed);
    deepSleep(syncFailed ? RETRY_SLEEP_SEC : SLEEP_DURATION_SEC);
}

void loop() {
    if (inSetupMode) {
        portalLoop();
    }
}
