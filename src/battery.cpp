#include "battery.h"
#include <Arduino.h>
#include "config.h"

int batteryReadMillivolts() {
    pinMode(BATTERY_CTRL_PIN, OUTPUT);
    digitalWrite(BATTERY_CTRL_PIN, HIGH);
    delay(10);

    analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_2_5db);
    // Average several reads to smooth noise.
    uint32_t sum = 0;
    const int samples = 16;
    for (int i = 0; i < samples; i++) sum += analogReadMilliVolts(BATTERY_ADC_PIN);
    uint32_t adcMv = sum / samples;

    // Release the control pin so it doesn't burn current in deep sleep.
    digitalWrite(BATTERY_CTRL_PIN, LOW);
    pinMode(BATTERY_CTRL_PIN, INPUT);

    if (adcMv == 0) return -1;
    return (int)(adcMv * BATTERY_MULTIPLIER);
}

int batteryReadPercent() {
    int mv = batteryReadMillivolts();
    if (mv < 0) return -1;
    long pct = (long)(mv - BATTERY_EMPTY_MV) * 100 /
               (BATTERY_FULL_MV - BATTERY_EMPTY_MV);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return (int)pct;
}
