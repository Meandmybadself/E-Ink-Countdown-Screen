#!/bin/bash
set -e

BUILD_DIR=".pio/build/vision-master-e213"
POLL_INTERVAL=0.05

echo "Building..."
pio run

# Snapshot existing ports so we can detect the device as it appears.
before_ports=$(ls /dev/cu.usbmodem* 2>/dev/null || true)

echo "Waiting for device on /dev/cu.usbmodem*..."
PORT=""
while [ -z "$PORT" ]; do
    current_ports=$(ls /dev/cu.usbmodem* 2>/dev/null || true)
    # Prefer any newly-appeared port (device just plugged/reset).
    for p in $current_ports; do
        if ! echo "$before_ports" | grep -qx "$p"; then
            PORT="$p"
            break
        fi
    done
    # Fallback: if nothing was there before, take the first one that shows up.
    if [ -z "$PORT" ] && [ -z "$before_ports" ] && [ -n "$current_ports" ]; then
        PORT=$(echo "$current_ports" | head -n1)
    fi
    [ -z "$PORT" ] && sleep "$POLL_INTERVAL"
done
echo "Found $PORT"

ESPTOOL=~/.platformio/packages/tool-esptoolpy/esptool.py
PYTHON=~/.platformio/penv/bin/python

echo "Erasing flash..."
"$PYTHON" "$ESPTOOL" --chip esp32s3 --port "$PORT" --baud 921600 \
    --before usb_reset erase_flash

echo "Uploading..."
"$PYTHON" "$ESPTOOL" --chip esp32s3 --port "$PORT" --baud 921600 \
    --before usb_reset --after hard_reset \
    write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB \
    0x0000  "$BUILD_DIR/bootloader.bin" \
    0x8000  "$BUILD_DIR/partitions.bin" \
    0x10000 "$BUILD_DIR/firmware.bin"

echo "Done."
