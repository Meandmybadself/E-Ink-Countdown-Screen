#pragma once

// Returns battery percentage (0–100). Returns -1 on read failure.
int batteryReadPercent();

// Returns battery voltage in millivolts. Returns -1 on read failure.
int batteryReadMillivolts();
