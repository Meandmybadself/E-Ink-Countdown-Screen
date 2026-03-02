#pragma once

#include <Arduino.h>

bool storageIsConfigured();
bool storageLoadConfig(String &ssid, String &password, String &targetDate, String &tz);
void storageSaveConfig(const String &ssid, const String &password, const String &targetDate, const String &tz);
void storageClear();
void storageSaveDays(int days);
int storageLoadDays();
