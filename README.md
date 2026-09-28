# Black Beacon

A storm-bound lighthouse exploration playtest for Linux x86_64.

## INSTALL

```bash
curl -fsSL https://raw.githubusercontent.com/daan-net/black-beacon/main/install.sh | bash
```

Requires Bash, curl, Python 3, zstd and flock (util-linux). No Unreal Editor,
compiler, sudo, GitHub account or GitHub Actions is needed for a **public** release.
On Debian/Ubuntu, missing tools can be installed with
`sudo apt install curl python3 zstd util-linux`.
A working Vulkan driver and the game's runtime libraries are also required;
see [Linux tester notes](TESTER_LINUX.md). Allow approximately 4 GB free space
for download and staged extraction, in addition to an existing installation.

## RUN

```bash
black-beacon
```

A Black Beacon entry is also added to the application menu. If your shell does
not include `~/.local/bin` in PATH, run `~/.local/bin/black-beacon` or add
`export PATH="$HOME/.local/bin:$PATH"` to your shell profile.

## UPDATE

Close the game and run the same install command again. It selects the newest
published Linux release **including tester prereleases**, verifies its matching
SHA-256 checksum before extracting, and switches the active application version
atomically. Repeating an unchanged release does not download the archive again.
Only one application version remains after a successful update.

Application files live under `~/.local/share/black-beacon/versions/`, with
`current` pointing to the active version. Saves/config/logs live separately under
`~/.local/share/black-beacon/userdata/`; packaged `Saved` paths link there.
Native Unreal data such as `~/.config/Epic/BlackBeacon/` is left untouched.
The installer also migrates saves from the earlier flat local-test installation.
Developer `Saved` files in the archive are not installed over tester data.

## UNINSTALL

From a source checkout:

```bash
bash uninstall-black-beacon.sh
```

Or directly from GitHub:

```bash
curl -fsSL https://raw.githubusercontent.com/daan-net/black-beacon/main/uninstall-black-beacon.sh | bash
```

Application files and shortcuts are removed; saves/config are preserved.
To explicitly delete the installer's managed save/config data, run
`bash uninstall-black-beacon.sh --remove-saves`. Native Unreal user directories
outside the installation are retained even with that option.

## Development and release preparation

[Development guide](README_FIRST.md) · [Distribution validation](DISTRIBUTION.md) ·
[Prepared release](ReleaseNotes/linux-test1-r2.md)

No CI/Actions workflow is used. A private repository cannot provide the anonymous
curl command above; a public distribution-only repository can host just the
installer, documentation and release binaries while game source stays private.
