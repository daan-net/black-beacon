#!/usr/bin/env bash
# Standalone installer: safe to run through `curl -fsSL URL | bash`.
# Proposed publication target; confirm repository/visibility before publishing.
main() {
    set -euo pipefail
    local repository="${BLACK_BEACON_REPOSITORY:-daan-net/black-beacon}"
    [[ "$(uname -s)" == Linux && "$(uname -m)" == x86_64 ]] || {
        echo 'Black Beacon requires Linux x86_64.' >&2; return 1;
    }
    local tool
    for tool in curl python3 zstd flock; do
        command -v "$tool" >/dev/null || {
            echo "Missing required tool: $tool. Install it with your distribution package manager and retry." >&2
            return 1
        }
    done
    exec python3 - "$repository" "$@" <<'PY'
import contextlib
import fcntl
import filecmp
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import signal
import stat
import subprocess
import sys
import tarfile
import tempfile
from urllib.parse import urlsplit

REPO = sys.argv[1]
MARKER = '.black-beacon-installer-v1'
SAVES = ('BlackBeacon/Saved', 'Engine/Saved', 'Saved')
LEGACY = ('BlackBeacon', 'Engine', 'black-beacon.sh', 'BlackBeacon.sh',
          'README.txt', 'README.md', 'NOTICES.txt', 'Manifest_UFSFiles_Linux.txt',
          'Manifest_NonUFSFiles_Linux.txt', 'Manifest_DebugFiles_Linux.txt')


def fail(message):
    raise RuntimeError(message)


def curl(url, destination):
    result = subprocess.run(['curl', '--fail', '--silent', '--show-error', '--location',
        '--proto', '=https', '--proto-redir', '=https', '--retry', '3',
        '--connect-timeout', '20', '--max-time', '3600', '--output', str(destination), url])
    if result.returncode:
        fail('Download failed. Check the network, public repository/release access and GitHub rate limits.')


def latest_release(work):
    candidates = []
    for page in range(1, 101):
        path = work / 'releases.json'
        curl(f'https://api.github.com/repos/{REPO}/releases?per_page=100&page={page}', path)
        releases = json.loads(path.read_text())
        if not isinstance(releases, list):
            fail('GitHub did not return a release list.')
        for release in releases:
            if release.get('draft') or not release.get('published_at'):
                continue
            assets = release.get('assets', [])
            archives = [a for a in assets if re.fullmatch(
                r'BlackBeacon-linux-x86_64-[A-Za-z0-9][A-Za-z0-9._-]*\.tar\.zst', a.get('name', ''))
                and a.get('state') == 'uploaded']
            pairs = [(a, s) for a in archives for s in assets
                     if s.get('name') == a['name'] + '.sha256' and s.get('state') == 'uploaded']
            if pairs:
                if len(pairs) != 1:
                    fail('A Linux release has ambiguous archive/checksum assets.')
                candidates.append((release, *pairs[0]))
        if len(releases) < 100:
            break
    else:
        fail('Too many release pages; cannot safely determine the newest Linux build.')
    if not candidates:
        fail('No published Linux x86_64 release with a matching .sha256 asset was found.')
    release, archive, checksum = max(candidates, key=lambda item: item[0]['published_at'])
    tag = release.get('tag_name', '')
    if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._/-]{0,127}', tag):
        fail('Release tag is missing or invalid.')
    for asset in (archive, checksum):
        url = urlsplit(asset.get('browser_download_url', ''))
        prefix = f'/{REPO}/releases/download/'
        if url.scheme != 'https' or url.netloc != 'github.com' or not url.path.startswith(prefix):
            fail('Release asset URL does not belong to the configured GitHub repository.')
    return tag, archive, checksum


def extract(archive, destination):
    # Stream extraction permits only ordinary files/directories inside one package
    # root. No archive-controlled links, devices, ownership or special permissions.
    top = None
    seen = set()
    process = subprocess.Popen(['zstd', '-dc', '--', str(archive)], stdout=subprocess.PIPE)
    try:
        with tarfile.open(fileobj=process.stdout, mode='r|') as bundle:
            for member in bundle:
                parts = PurePosixPath(member.name).parts
                if (not parts or member.name.startswith('/') or '..' in parts
                        or any(ord(c) < 32 for c in member.name)
                        or not (member.isdir() or member.isfile())):
                    fail('Unsafe archive member: ' + repr(member.name))
                if top is None:
                    top = parts[0]
                if parts[0] != top:
                    fail('Archive must contain one top-level package directory.')
                relative = PurePosixPath(*parts[1:])
                if not parts[1:]:
                    if not member.isdir():
                        fail('Archive package root is not a directory.')
                    continue
                # Do not install the developer machine's Saved logs/config/screenshots.
                if any(relative == PurePosixPath(s) or PurePosixPath(s) in relative.parents for s in SAVES):
                    continue
                if relative in seen:
                    fail('Archive contains a duplicate path: ' + str(relative))
                seen.add(relative)
                target = destination.joinpath(*parts[1:])
                if member.isdir():
                    target.mkdir(parents=True, exist_ok=True)
                else:
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with bundle.extractfile(member) as source, target.open('xb') as output:
                        shutil.copyfileobj(source, output, 1024 * 1024)
                    target.chmod(0o755 if member.mode & 0o111 else 0o644)
        # Consume tar padding/trailing bytes so zstd cannot block on a full pipe.
        while process.stdout.read(1024 * 1024):
            pass
        if process.wait() != 0:
            fail('Archive decompression failed.')
    finally:
        process.stdout.close()
        if process.poll() is None:
            process.terminate()
        process.wait()
    if not valid_payload(destination):
        fail('Verified archive is missing its executable game or black-beacon.sh launcher.')


def valid_payload(destination):
    launcher = destination / 'black-beacon.sh'
    binaries = destination / 'BlackBeacon/Binaries/Linux'
    return launcher.is_file() and os.access(launcher, os.X_OK) and any(
            (binaries / name).is_file() and os.access(binaries / name, os.X_OK)
            for name in ('BlackBeacon', 'BlackBeacon-Linux-Development', 'BlackBeacon-Linux-Shipping'))


def atomic_text(path, text, mode=0o644):
    descriptor, temporary = tempfile.mkstemp(prefix='.black-beacon-', dir=path.parent)
    try:
        with os.fdopen(descriptor, 'w') as stream:
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
        os.chmod(temporary, mode)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def desktop_quote(value):
    # Exec has a desktop-entry escape layer followed by argument parsing.
    value = value.replace('%', '%%')
    for char in ('\\', '"', '`', '$'):
        value = value.replace(char, '\\' + char)
    return '"' + value.replace('\\', '\\\\') + '"'


def install():
    if sys.argv[2:]:
        fail('Usage: bash install.sh (no arguments).')
    if REPO == 'OWNER/REPO' or not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', REPO):
        fail('Publication repository is not configured. Set BLACK_BEACON_REPOSITORY=OWNER/REPO or use the published installer.')
    home_value = os.environ.get('HOME', '')
    if not home_value.startswith('/') or any(ord(c) < 32 for c in home_value):
        fail('HOME must be an absolute directory without control characters.')
    home = Path(home_value)
    base = home / '.local/share'
    root = base / 'black-beacon'
    bin_dir = home / '.local/bin'
    apps = base / 'applications'
    base.mkdir(parents=True, exist_ok=True)
    lock = (base / '.black-beacon-install.lock').open('a')
    try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        fail('Black Beacon or another installer is running. Close it and retry.')
    if root.is_symlink():
        fail('Installation root is a symlink; refusing to replace it.')
    legacy = root.exists() and not (root / MARKER).exists()
    if legacy and any(root.iterdir()) and not ((root / 'black-beacon.sh').is_file()
            or (root / 'BlackBeacon/Saved').is_dir()):
        fail('Installation directory contains unrecognized data; no files were changed.')
    if (root / MARKER).exists() and (root / MARKER).read_text().strip() != REPO:
        fail('Existing installation belongs to a different repository.')
    for path in (root / 'versions', root / 'userdata', root / 'current'):
        if path.is_symlink() and path.name != 'current':
            fail('Managed installation directory unexpectedly points outside the installation.')
    for path in (bin_dir / 'black-beacon', apps / 'black-beacon.desktop'):
        if path.exists() and not path.is_file():
            fail('Launcher path is not a regular file: ' + str(path))
        if path.exists() and 'black-beacon' not in path.read_text(errors='replace').lower():
            fail('Refusing to overwrite an unrelated launcher: ' + str(path))
    with tempfile.TemporaryDirectory(prefix='.black-beacon-download-', dir=base) as temporary, contextlib.ExitStack() as cleanup:
        work = Path(temporary)
        tag, archive_asset, checksum_asset = latest_release(work)
        checksum_file = work / 'checksum'
        curl(checksum_asset['browser_download_url'], checksum_file)
        match = re.fullmatch(r'([0-9a-fA-F]{64}) [ *]' + re.escape(archive_asset['name']) + r'\n?',
                             checksum_file.read_text())
        if not match:
            fail('Checksum must contain exactly one SHA-256 entry for the selected archive.')
        digest = match[1].lower()
        version = root / 'versions' / digest
        current = root / 'current'
        same = current.is_symlink() and current.resolve() == version and (version / '.release.json').is_file() and valid_payload(version)
        if not same:
            archive = work / archive_asset['name']
            print('Downloading Black Beacon ' + tag + '...', flush=True)
            curl(archive_asset['browser_download_url'], archive)
            hasher = hashlib.sha256()
            with archive.open('rb') as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b''):
                    hasher.update(block)
            if hasher.hexdigest() != digest:
                fail('SHA-256 mismatch. Existing application and saves were not changed.')
            staged = work / 'payload'
            staged.mkdir()
            print('SHA-256 verified. Extracting...', flush=True)
            extract(archive, staged)
        root.mkdir(exist_ok=True)
        for relative in SAVES:
            data = root / 'userdata' / relative
            if data.is_symlink():
                fail('Saved-data directory is unexpectedly a symlink: ' + str(data))
            data.mkdir(parents=True, exist_ok=True)
            old = root / relative
            if legacy and old.is_dir():
                # Copy before switching; never delete the only copy on a failed update.
                def preserve_copy(source, target):
                    if os.path.exists(target):
                        if not filecmp.cmp(source, target, shallow=False):
                            fail('Conflicting legacy and retained save data: ' + str(target))
                        return target
                    return shutil.copy2(source, target)
                shutil.copytree(old, data, dirs_exist_ok=True, copy_function=preserve_copy)
            if not same:
                link = staged / relative
                link.parent.mkdir(parents=True, exist_ok=True)
                link.symlink_to(os.path.relpath(data, version / Path(relative).parent))
        (root / 'versions').mkdir(exist_ok=True)
        bin_dir.mkdir(parents=True, exist_ok=True)
        apps.mkdir(parents=True, exist_ok=True)
        metadata = json.dumps({'tag': tag, 'sha256': digest, 'asset': archive_asset['name']}, indent=2) + '\n'
        if not same:
            (staged / '.release.json').write_text(metadata)
            if version.exists():
                shutil.rmtree(version)  # Inactive leftover from an interrupted install.
            os.replace(staged, version)
            def discard_inactive_version():
                if not (current.is_symlink() and current.resolve() == version):
                    shutil.rmtree(version, ignore_errors=True)
            cleanup.callback(discard_inactive_version)
        launcher = '''#!/usr/bin/env bash
# black-beacon managed launcher; keep the selected package in use during play.
set -euo pipefail
exec flock --shared "$HOME/.local/share/.black-beacon-install.lock" \\
    "$HOME/.local/share/black-beacon/current/black-beacon.sh" "$@"
'''
        desktop = '[Desktop Entry]\nType=Application\nName=Black Beacon\nComment=Storm lighthouse exploration\nExec=' + desktop_quote(str(bin_dir / 'black-beacon')) + '\nIcon=applications-games\nTerminal=false\nCategories=Game;\n'
        paths = (current, bin_dir / 'black-beacon', apps / 'black-beacon.desktop', root / MARKER, version / '.release.json')
        backups = []
        for path in paths:
            if path.is_symlink():
                backups.append(('link', os.readlink(path), None))
            elif path.is_file():
                backups.append(('file', path.read_text(), stat.S_IMODE(path.stat().st_mode)))
            elif path.exists():
                fail('Cannot replace non-file installation path: ' + str(path))
            else:
                backups.append(('missing', None, None))
        changed = []
        try:
            # Same-filesystem symlink rename is the application commit point.
            pointer = work / 'current'
            pointer.symlink_to('versions/' + digest)
            os.replace(pointer, current)
            changed.append(0)
            atomic_text(bin_dir / 'black-beacon', launcher, 0o755)
            changed.append(1)
            atomic_text(apps / 'black-beacon.desktop', desktop)
            changed.append(2)
            atomic_text(root / MARKER, REPO + '\n')
            changed.append(3)
            atomic_text(version / '.release.json', metadata)
            changed.append(4)
        except BaseException:
            rollback_errors = []
            for index in reversed(changed):
                path = paths[index]
                kind, value, mode = backups[index]
                try:
                    if kind == 'file':
                        atomic_text(path, value, mode)
                    elif kind == 'link':
                        replacement = work / 'rollback'
                        replacement.symlink_to(value)
                        os.replace(replacement, path)
                    else:
                        path.unlink(missing_ok=True)
                except OSError as error:
                    rollback_errors.append(str(error))
            if rollback_errors:
                print('Rollback needs attention: ' + '; '.join(rollback_errors), file=sys.stderr)
            raise
        # Cleanup never touches userdata or native Unreal data outside this root.
        for old in (root / 'versions').iterdir():
            if old != version:
                if old.is_symlink() or old.is_file(): old.unlink()
                else: shutil.rmtree(old)
        if legacy:
            for name in LEGACY:
                old = root / name
                if old.is_symlink() or old.is_file(): old.unlink()
                elif old.is_dir(): shutil.rmtree(old)
        print(('Already installed: ' if same else 'Installed: ') + 'Black Beacon ' + tag)
        print('Run: black-beacon')
        if str(bin_dir) not in os.environ.get('PATH', '').split(os.pathsep):
            print('Your PATH does not include ~/.local/bin. Add this to your shell profile:')
            print('  export PATH="$HOME/.local/bin:$PATH"')
            print('For now, run: ' + str(bin_dir / 'black-beacon'))
        print('Saves/config preserved in ' + str(root / 'userdata') + ' and any native Unreal user directories.')

def interrupted(signum, frame):
    raise InterruptedError('Interrupted; temporary downloads are being cleaned up.')


for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
    signal.signal(signum, interrupted)

try:
    install()
except (Exception, KeyboardInterrupt) as error:
    print('Black Beacon installation failed: ' + str(error), file=sys.stderr)
    sys.exit(1)
PY
}
main "$@"
