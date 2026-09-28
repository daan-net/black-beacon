#!/usr/bin/env bash
main() {
    set -euo pipefail
    command -v python3 >/dev/null || { echo 'python3 is required.' >&2; return 1; }
    exec python3 - "$@" <<'PY'
import fcntl
import os
from pathlib import Path
import shutil
import sys


def remove(path):
    if path.is_symlink() or path.is_file():
        path.unlink()
    elif path.is_dir():
        shutil.rmtree(path)


def main():
    if sys.argv[1:] not in ([], ['--remove-saves']):
        raise RuntimeError('Usage: uninstall-black-beacon.sh [--remove-saves]')
    home_value = os.environ.get('HOME', '')
    if not home_value.startswith('/') or any(ord(c) < 32 for c in home_value):
        raise RuntimeError('HOME must be an absolute directory without control characters.')
    home = Path(home_value)
    base = home / '.local/share'
    root = base / 'black-beacon'
    if not root.exists() and not root.is_symlink():
        print('Black Beacon is not installed. User data was not changed.')
        return
    if root.is_symlink() or not (root / '.black-beacon-installer-v1').is_file():
        raise RuntimeError('Not a managed Black Beacon installation; no files were removed. For a legacy local installation use Tools/uninstall-test-build.sh.')
    lock = (base / '.black-beacon-install.lock').open('a')
    try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        raise RuntimeError('Black Beacon or another installer is running. Close it and retry.')
    remove(root / 'current')
    remove(root / 'versions')
    for path in (home / '.local/bin/black-beacon', base / 'applications/black-beacon.desktop'):
        if path.is_file() and 'black-beacon' in path.read_text(errors='replace').lower():
            path.unlink()
    if '--remove-saves' in sys.argv:
        remove(root / 'userdata')
        print('Removed managed saves/config (--remove-saves).')
    else:
        print('Saves/config preserved at ' + str(root / 'userdata'))
    # Keep the marker so reinstall recognizes retained data. Unknown files and
    # native Unreal ~/.config/Epic/BlackBeacon data are never recursively purged.
    print('Black Beacon uninstalled. Native Unreal user directories were left untouched.')

try:
    main()
except (Exception, KeyboardInterrupt) as error:
    print('Black Beacon uninstall failed: ' + str(error), file=sys.stderr)
    sys.exit(1)
PY
}
main "$@"
