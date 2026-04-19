## Process

After updating code:

```bash
./upload.sh    # builds, waits for device, flashes
./monitor.sh   # waits for device, opens serial monitor
```

### Entering download mode

The ESP32-S3 uses native USB-CDC. Reliable sequence:

1. Unplug USB
2. Hold **BOOT** + **RST** together
3. Plug USB back in while holding both
4. Release **RST** (keep holding BOOT ~1s, then release)

`upload.sh` uses `--before usb_reset`, so a plain replug may also work — but the BOOT+RST hold is the guaranteed path if a board is stuck.

Press **RST** alone to see a fresh boot log in `monitor.sh`.
