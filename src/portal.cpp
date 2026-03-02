#include "portal.h"
#include "config.h"
#include "storage.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <time.h>

static WebServer server(80);
static DNSServer dnsServer;

static const char FORM_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Countdown Setup</title>
<style>
* { box-sizing: border-box; margin: 0; padding: 0; }
body { font-family: -apple-system, system-ui, sans-serif; background: #f5f5f5; padding: 20px; }
.card { max-width: 400px; margin: 0 auto; background: #fff; border-radius: 12px; padding: 24px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }
h1 { font-size: 22px; margin-bottom: 20px; text-align: center; }
label { display: block; font-weight: 600; margin-bottom: 4px; margin-top: 16px; font-size: 14px; }
input, select { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 8px; font-size: 16px; }
input:focus, select:focus { outline: none; border-color: #007aff; }
button { width: 100%; margin-top: 24px; padding: 12px; background: #007aff; color: #fff; border: none; border-radius: 8px; font-size: 16px; font-weight: 600; cursor: pointer; }
button:active { background: #005ec4; }
.note { color: #888; font-size: 12px; margin-top: 4px; }
</style>
</head>
<body>
<div class="card">
<h1>Countdown Setup</h1>
<form method="POST" action="/save">
  <label for="ssid">WiFi Network</label>
  <input type="text" id="ssid" name="ssid" maxlength="32" required placeholder="Your WiFi name">

  <label for="password">WiFi Password</label>
  <input type="password" id="password" name="password" maxlength="64" placeholder="Your WiFi password">

  <label for="target_date">Target Date</label>
  <input type="date" id="target_date" name="target_date" required>

  <label for="tz">Timezone</label>
  <select id="tz" name="tz">
    <option value="EST5EDT,M3.2.0,M11.1.0">Eastern (US)</option>
    <option value="CST6CDT,M3.2.0,M11.1.0">Central (US)</option>
    <option value="MST7MDT,M3.2.0,M11.1.0">Mountain (US)</option>
    <option value="PST8PDT,M3.2.0,M11.1.0">Pacific (US)</option>
    <option value="MST7">Arizona (no DST)</option>
    <option value="AKST9AKDT,M3.2.0,M11.1.0">Alaska</option>
    <option value="HST10">Hawaii</option>
    <option value="GMT0BST,M3.5.0/1,M10.5.0">UK</option>
    <option value="CET-1CEST,M3.5.0,M10.5.0/3">Central Europe</option>
    <option value="EET-2EEST,M3.5.0/3,M10.5.0/4">Eastern Europe</option>
    <option value="JST-9">Japan</option>
    <option value="AEST-10AEDT,M10.1.0,M4.1.0/3">Australia Eastern</option>
    <option value="UTC0">UTC</option>
  </select>

  <button type="submit">Save &amp; Connect</button>
</form>
</div>
</body>
</html>
)rawliteral";

static const char SUCCESS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Success</title>
<style>
body { font-family: -apple-system, system-ui, sans-serif; background: #f5f5f5; padding: 20px; text-align: center; }
.card { max-width: 400px; margin: 40px auto; background: #fff; border-radius: 12px; padding: 24px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }
h1 { color: #34c759; margin-bottom: 12px; }
</style>
</head>
<body>
<div class="card">
<h1>Success!</h1>
<p>Configuration saved. The device will now reboot and start counting down.</p>
<p style="margin-top:12px;color:#888;">You can disconnect from the Countdown WiFi network.</p>
</div>
</body>
</html>
)rawliteral";

static String errorPage(const char* message) {
    String html = F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Error</title>"
        "<style>"
        "body { font-family: -apple-system, system-ui, sans-serif; background: #f5f5f5; padding: 20px; text-align: center; }"
        ".card { max-width: 400px; margin: 40px auto; background: #fff; border-radius: 12px; padding: 24px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }"
        "h1 { color: #ff3b30; margin-bottom: 12px; }"
        "a { display: inline-block; margin-top: 16px; padding: 10px 24px; background: #007aff; color: #fff; text-decoration: none; border-radius: 8px; }"
        "</style></head><body><div class='card'><h1>Error</h1><p>");
    html += message;
    html += F("</p><a href='/'>Try Again</a></div></body></html>");
    return html;
}

static void handleRoot() {
    server.send(200, "text/html", FORM_HTML);
}

static void handleSave() {
    String ssid       = server.arg("ssid");
    String password   = server.arg("password");
    String targetDate = server.arg("target_date");
    String tz         = server.arg("tz");

    if (ssid.length() == 0 || targetDate.length() == 0 || tz.length() == 0) {
        server.send(400, "text/html", errorPage("Please fill in all required fields."));
        return;
    }

    Serial.printf("Testing WiFi: %s\n", ssid.c_str());

    // Switch to AP+STA to test credentials while keeping portal alive
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_TIMEOUT_MS) {
            Serial.println("WiFi test FAILED");
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP);
            server.send(200, "text/html", errorPage("Could not connect to that WiFi network. Check your SSID and password."));
            return;
        }
        delay(250);
    }
    Serial.println("WiFi test OK");

    // Stop DNS server so NTP can resolve pool.ntp.org via the real network
    dnsServer.stop();

    // Test NTP sync
    configTzTime(tz.c_str(), NTP_SERVER);
    struct tm timeinfo;
    bool synced = false;
    for (int i = 0; i < 20; i++) {
        if (getLocalTime(&timeinfo, 1000)) {
            synced = true;
            break;
        }
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    // Restart DNS server for captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());

    if (!synced) {
        Serial.println("NTP sync FAILED during setup");
        server.send(200, "text/html", errorPage("WiFi connected but time sync failed. Please try again."));
        return;
    }

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("NTP sync OK: %s\n", buf);

    // Save config
    storageSaveConfig(ssid, password, targetDate, tz);
    Serial.println("Config saved to NVS");

    // Send success page, then reboot
    server.send(200, "text/html", SUCCESS_HTML);
    delay(3000);
    ESP.restart();
}

static void handleNotFound() {
    // Captive portal: redirect everything to root
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
}

void portalStart() {
    Serial.println("Starting setup mode...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    delay(100);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // DNS: redirect all domains to our IP (captive portal detection)
    dnsServer.start(53, "*", WiFi.softAPIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Web server started on port 80");
}

void portalLoop() {
    dnsServer.processNextRequest();
    server.handleClient();
}
