#include "display.h"
#include <heltec-eink-modules.h>
#include "number60pt7b.h"
#include "number10pt7b.h"

static EInkDisplay_VisionMasterE213V1_1 display;

void displayInit() {
    // Display initializes in constructor; nothing extra needed
}

void displayShowCountdown(int days, int targetYear, int targetMonth, int targetDay, bool syncIndicator) {
    display.landscape();
    display.clear();
    display.setTextColor(BLACK);

    // Large days number, centered upper area
    String daysStr = String(days);
    display.setFont(&number60pt7b);
    int numW = display.getTextWidth(daysStr.c_str());
    int numX = (display.width() - numW) / 2;
    display.setCursor(numX, 88);
    display.print(daysStr);

    // Target date label
    display.setFont(&number10pt7b);
    const char* months[] = {"JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE",
                            "JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER"};
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%s %d, %d", months[targetMonth - 1], targetDay, targetYear);
    int dateW = display.getTextWidth(dateBuf);
    display.setCursor((display.width() - dateW) / 2, 114);
    display.print(dateBuf);

    // Tiny sync indicator dot in bottom-right corner
    if (syncIndicator) {
        display.fillRect(display.width() - 6, display.height() - 6, 4, 4, BLACK);
    }

    display.update();
}

void displayShowSetupMode() {
    display.landscape();
    display.clear();
    display.setTextColor(BLACK);
    display.setFont(&number10pt7b);

    display.setCursor(10, 30);
    display.print("SETUP MODE");

    display.setCursor(10, 60);
    display.print("WiFi: Countdown");

    display.setCursor(10, 90);
    display.print("Go to: 192.168.4.1");

    display.update();
}
