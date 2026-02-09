#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <heltec-eink-modules.h>
#include "Fonts/FreeSansBold24pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

#include "config.h"

EInkDisplay_VisionMasterE213V1_1 display;

RTC_DATA_ATTR int bootCount = 0;

bool connectWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

bool syncTime() {
    Serial.println("Syncing time with NTP server...");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    struct tm timeinfo;
    int retries = 10;
    while (!getLocalTime(&timeinfo) && retries > 0) {
        delay(500);
        retries--;
    }

    if (retries == 0) {
        Serial.println("NTP sync FAILED!");
        return false;
    }

    Serial.println("Time synchronized!");
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("Current time: ");
    Serial.println(buf);
    return true;
}

// Convert a date to a day count (consistent reference, no timezone dependency)
long dateToDays(int y, int m, int d) {
    if (m <= 2) { y--; m += 12; }
    return 365L * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + d;
}

int calculateDaysRemaining() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return -1;

    int nowY = timeinfo.tm_year + 1900;
    int nowM = timeinfo.tm_mon + 1;
    int nowD = timeinfo.tm_mday;

    long today  = dateToDays(nowY, nowM, nowD);
    long target = dateToDays(TARGET_YEAR, TARGET_MONTH, TARGET_DAY);
    long diff   = target - today;

    Serial.printf("Today: %04d-%02d-%02d (%ld), Target: %04d-%02d-%02d (%ld), Diff: %ld\n",
                  nowY, nowM, nowD, today, TARGET_YEAR, TARGET_MONTH, TARGET_DAY, target, diff);

    return (diff > 0) ? (int)diff : 0;
}

void showCountdown(int days) {
    display.landscape();
    display.clear();
    display.setTextColor(BLACK);

    // Large days number, centered upper area
    String daysStr = String(days);
    display.setFont(&FreeSansBold24pt7b);
    int numW = display.getTextWidth(daysStr.c_str());
    int numX = (display.width() - numW) / 2;
    display.setCursor(numX, 58);
    display.print(daysStr);

    // "days until" label
    display.setFont(&FreeSans9pt7b);
    const char* label = "days until";
    int labelW = display.getTextWidth(label);
    display.setCursor((display.width() - labelW) / 2, 82);
    display.print(label);

    // Target date
    const char* months[] = {"January","February","March","April","May","June",
                            "July","August","September","October","November","December"};
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%s %d, %d", months[TARGET_MONTH - 1], TARGET_DAY, TARGET_YEAR);
    int dateW = display.getTextWidth(dateBuf);
    display.setCursor((display.width() - dateW) / 2, 102);
    display.print(dateBuf);

    display.update();
    Serial.println("Display updated!");
}

void showError(const char* message) {
    display.landscape();
    display.clear();
    display.setTextColor(BLACK);

    display.setFont(&FreeSans9pt7b);
    display.printCenter(message);
    display.update();
}

void enterDeepSleep() {
    Serial.printf("Entering deep sleep for %d seconds...\n", SLEEP_DURATION);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_DURATION * 1000000ULL);
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    delay(100);

    bootCount++;
    Serial.printf("\nBoot count: %d\n", bootCount);

    if (!connectWiFi()) {
        showError("WiFi Error");
        enterDeepSleep();
        return;
    }

    if (!syncTime()) {
        showError("Sync Failed");
        enterDeepSleep();
        return;
    }

    int days = calculateDaysRemaining();
    if (days < 0) {
        showError("Time Error");
        enterDeepSleep();
        return;
    }

    Serial.printf("Days until %04d-%02d-%02d: %d\n", TARGET_YEAR, TARGET_MONTH, TARGET_DAY, days);

    showCountdown(days);
    enterDeepSleep();
}

void loop() {
    // Never reached — device deep sleeps after setup()
}
