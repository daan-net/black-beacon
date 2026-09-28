#!/usr/bin/env bash
set -euo pipefail

# Black Beacon Installer for Linux

# Find the archive in dist/
ARCHIVE_NAME="BlackBeacon-linux-x86_64-test1.tar.zst"
DIST_DIR="$(cd "$(dirname "$0")/../dist" && pwd)"
ARCHIVE_PATH="$DIST_DIR/$ARCHIVE_NAME"
CHECKSUM_PATH="$ARCHIVE_PATH.sha256"

if [ ! -f "$ARCHIVE_PATH" ]; then
    echo "Error: Archive not found at $ARCHIVE_PATH"
    exit 1
fi

if [ ! -f "$CHECKSUM_PATH" ]; then
    echo "Error: Checksum file not found at $CHECKSUM_PATH"
    exit 1
fi

echo "Verifying checksum..."
cd "$DIST_DIR"
if ! sha256sum -c "$CHECKSUM_PATH"; then
    echo "Error: Checksum validation failed!"
    exit 1
fi

INSTALL_DIR="$HOME/.local/share/black-beacon"
BIN_DIR="$HOME/.local/bin"
DESKTOP_DIR="$HOME/.local/share/applications"

echo "Installing to $INSTALL_DIR..."

# Safe reinstall/update: do not touch Saved/ (which holds saves/config)
if [ -d "$INSTALL_DIR" ]; then
    echo "Existing installation found. Updating..."
    # We want to remove everything EXCEPT the Saved/ directory.
    # The Saved directory in Unreal is typically Engine/Saved or ProjectName/Saved.
    # In a packaged build, it's usually BlackBeacon/Saved.
    find "$INSTALL_DIR" -mindepth 1 -maxdepth 1 ! -name 'BlackBeacon' -exec rm -rf {} +
    if [ -d "$INSTALL_DIR/BlackBeacon" ]; then
        find "$INSTALL_DIR/BlackBeacon" -mindepth 1 -maxdepth 1 ! -name 'Saved' -exec rm -rf {} +
    fi
else
    mkdir -p "$INSTALL_DIR"
fi

echo "Extracting archive..."
tar --zstd -xf "$ARCHIVE_PATH" -C "$INSTALL_DIR" --strip-components=1

echo "Setting up launcher..."
mkdir -p "$BIN_DIR"
cat > "$BIN_DIR/black-beacon" << 'EOF'
#!/usr/bin/env bash
set -euo pipefail
EXEC_PATH="$HOME/.local/share/black-beacon/black-beacon.sh"
if [ ! -f "$EXEC_PATH" ]; then
    echo "Error: Game launcher not found at $EXEC_PATH"
    exit 1
fi
exec "$EXEC_PATH" "$@"
EOF
chmod +x "$BIN_DIR/black-beacon"

echo "Creating desktop entry..."
mkdir -p "$DESKTOP_DIR"
cat > "$DESKTOP_DIR/black-beacon.desktop" << EOF
[Desktop Entry]
Name=Black Beacon
Exec=$BIN_DIR/black-beacon
Type=Application
Categories=Game;
Terminal=false
EOF

echo "Installation complete!"
echo "You can launch the game by typing 'black-beacon' or from your application menu."
