#include "storage.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

bool storageIsConfigured() {
    prefs.begin(NVS_NAMESPACE, true);
    uint8_t val = prefs.getUChar(NVS_KEY_CONFIGURED, 0);
    prefs.end();
    return val == 1;
}

bool storageLoadConfig(String &ssid, String &password, String &targetDate, String &tz) {
    prefs.begin(NVS_NAMESPACE, true);
    ssid       = prefs.getString(NVS_KEY_SSID, "");
    password   = prefs.getString(NVS_KEY_PASSWORD, "");
    targetDate = prefs.getString(NVS_KEY_TARGET, "");
    tz         = prefs.getString(NVS_KEY_TZ, "");
    prefs.end();
    return ssid.length() > 0 && targetDate.length() > 0 && tz.length() > 0;
}

void storageSaveConfig(const String &ssid, const String &password, const String &targetDate, const String &tz) {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString(NVS_KEY_SSID, ssid);
    prefs.putString(NVS_KEY_PASSWORD, password);
    prefs.putString(NVS_KEY_TARGET, targetDate);
    prefs.putString(NVS_KEY_TZ, tz);
    prefs.putUChar(NVS_KEY_CONFIGURED, 1);
    prefs.end();
}

void storageClear() {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.clear();
    prefs.end();
}

void storageSaveDays(int days) {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putInt("days_left", days);
    prefs.end();
}

int storageLoadDays() {
    prefs.begin(NVS_NAMESPACE, true);
    int val = prefs.getInt("days_left", -1);
    prefs.end();
    return val;
}
