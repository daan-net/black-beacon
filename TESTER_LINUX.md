# Black Beacon Linux Tester Guide

Welcome to the Linux tester program for Black Beacon! This guide covers everything you need to know about installing, running, and managing your local playtest build.

## Minimum Practical Requirements
- **OS**: Linux (x86_64), modern distribution (Ubuntu 22.04+, Fedora 38+, Arch, etc.)
- **Storage**: ~1.7 GB free space (1.2 GB unpacked game + 480 MB installer)
- **Dependencies**: No external engine or compiler is required. Standard Vulkan and X11/Wayland libraries are needed to run the executable.

## Installation

You have been provided with an installation archive (e.g. `BlackBeacon-linux-x86_64-test1.tar.zst`) and its checksum file (`.sha256`). Ensure both files are in the same directory.

1. Open a terminal.
2. Run the provided installer script:
   ```bash
   ./Tools/install-test-build.sh
   ```
   The installer will:
   - Verify the package integrity (SHA-256).
   - Extract the game files to `~/.local/share/black-beacon`.
   - Create a launcher shortcut `black-beacon` in `~/.local/bin`.
   - Add a desktop entry to your application menu.

## Running the Game

You can run the game using one of two methods:
- **Terminal**: Type `black-beacon` and press Enter.
- **Desktop Application**: Open your system's application launcher and search for "Black Beacon".

## Updating the Game

When a new package is released, place the new archive and its `.sha256` checksum in the distribution directory and run the installer script again:
```bash
./Tools/install-test-build.sh
```
The installer is designed for safe updates. It will replace the application files while strictly preserving your saved games and configurations.

## Uninstallation

To safely remove the game and all shortcuts while keeping your saved data:
```bash
./Tools/uninstall-test-build.sh
```

If you wish to completely remove everything, including your saved games and local configurations, use the `--purge` flag:
```bash
./Tools/uninstall-test-build.sh --purge
```

## Save Data & Configurations
Your saved games and local user configurations are securely stored inside your installation folder, separated from the executable updates:
- **Location**: `~/.local/share/black-beacon/BlackBeacon/Saved`

## Log Location
If the game crashes or you experience unusual bugs, logs will be generated in:
- **Logs**: `~/.local/share/black-beacon/BlackBeacon/Saved/Logs`

## Bug Reporting
When reporting issues, please include:
1. Steps to reproduce the bug.
2. The latest `.log` file from your log location.
3. System specifications (GPU, Distribution, RAM).
4. The test package name (e.g., `BlackBeacon-linux-x86_64-test1`).
