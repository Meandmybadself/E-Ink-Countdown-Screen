## Process

After updating code:

```bash
./upload.sh    # builds, waits for device, flashes
./monitor.sh   # waits for device, opens serial monitor
```

### Entering download mode

The ESP32-S3 uses native USB-CDC. To flash reliably:

1. Unplug USB cable
2. Hold **BUTTON 1** (GPIO 0 / BOOT)
3. Plug USB back in while holding BUTTON 1
4. Release BUTTON 1 after ~1s

`upload.sh` uses `--before usb_reset`, so a plain replug often works too — but the BUTTON 1 hold is the guaranteed path if a board is stuck.

Press **RST** to see a fresh boot log in `monitor.sh`.
