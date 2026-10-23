#!/usr/bin/env bash
set -euo pipefail
echo "Compiling & Flashing Glorious Engrammer for Dygma Defy 70..."
arduino-cli compile --fqbn dygma:rp2040:defy --output-dir ./build .
PORT=$(arduino-cli board list | awk '/35ef/ {print $1; exit}')
if [ -n "${PORT:-}" ]; then
  arduino-cli upload -p "$PORT" --fqbn dygma:rp2040:defy --input-dir ./build
else
  echo "Compiled .uf2 available in ./build/ — copy to RPI-RP2 drive or flash from the web studio."
fi
