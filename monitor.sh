#!/bin/bash
set -e

POLL_INTERVAL=0.05

echo "Waiting for device on /dev/cu.usbmodem*..."
while :; do
    PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1 || true)
    [ -n "$PORT" ] && break
    sleep "$POLL_INTERVAL"
done

echo "Found $PORT — starting monitor (Ctrl+C to exit)"
exec pio device monitor --port "$PORT" --baud 115200
