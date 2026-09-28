#!/usr/bin/env bash
set -euo pipefail

# Black Beacon Uninstaller for Linux

INSTALL_DIR="$HOME/.local/share/black-beacon"
BIN_DIR="$HOME/.local/bin"
DESKTOP_DIR="$HOME/.local/share/applications"

REMOVE_SAVES=0

for arg in "$@"; do
    if [ "$arg" == "--remove-saves" ] || [ "$arg" == "--purge" ]; then
        REMOVE_SAVES=1
    fi
done

echo "Uninstalling Black Beacon..."

if [ -d "$INSTALL_DIR" ]; then
    if [ $REMOVE_SAVES -eq 1 ]; then
        echo "Removing game and saved data..."
        rm -rf "$INSTALL_DIR"
    else
        echo "Removing game files (preserving saved data)..."
        # Remove everything except BlackBeacon/Saved
        find "$INSTALL_DIR" -mindepth 1 -maxdepth 1 ! -name 'BlackBeacon' -exec rm -rf {} +
        if [ -d "$INSTALL_DIR/BlackBeacon" ]; then
            find "$INSTALL_DIR/BlackBeacon" -mindepth 1 -maxdepth 1 ! -name 'Saved' -exec rm -rf {} +
            # If BlackBeacon is empty except for Saved, leave it. If completely empty, remove it.
        fi
    fi
else
    echo "Game not found at $INSTALL_DIR."
fi

echo "Removing launcher and desktop entry..."
rm -f "$BIN_DIR/black-beacon"
rm -f "$DESKTOP_DIR/black-beacon.desktop"

echo "Uninstallation complete!"
