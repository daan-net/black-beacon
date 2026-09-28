# Linux installation infrastructure — 2026-09-28

**IMPLEMENTED / TESTED locally. Public publication approved; upload/verification pending.**
No game rebuild, recook, repackaging, gameplay/art edits or CI/Actions were performed.
The existing validated `dist/BlackBeacon-linux-x86_64-test1-r2.tar.zst` and its
existing checksum are the release inputs. SHA-256 verification succeeded:

```text
583419c562d3e07b7b2fdfd42fb136a885896020a4348649c91614f7e2dc7df4
```

## Installer contract

`install.sh` is a self-contained Bash entry point with Python standard-library
implementation. Its complete function is parsed before invocation, supporting
`curl ... | bash`. Requires Linux x86_64, Bash, curl, Python 3, zstd and flock.
These are distribution-tool dependencies only; no game dependencies were added.

- Queries paginated GitHub Releases and selects the latest `published_at` among
  published releases containing exactly one Black Beacon Linux x86_64 archive
  and matching `.sha256`. Includes prereleases for the playtest channel; ignores
  drafts/releases without matching Linux assets. Ambiguous asset pairs fail.
- HTTPS-only curl downloads, clear HTTP/rate-limit/dependency/platform errors.
  Validates checksum filename and SHA-256 before extraction or application changes.
- Streams zstd/tar into an isolated same-filesystem stage; rejects traversal,
  absolute paths, symlinks, hardlinks, devices and duplicate entries. Executable
  launcher and game binary are required. Special permission/ownership bits are
  not imported. This release's archive contains ordinary files/directories only.
- Installs under `~/.local/share/black-beacon/versions/<sha256>`; atomically
  replaces the `current` symlink, with rollback for launcher/desktop transaction
  errors. Removes older application versions after a successful switch.
- Keeps all three packaged Saved paths (`BlackBeacon/Saved`, `Engine/Saved`,
  `Saved`) linked into persistent `userdata`. Never imports developer Saved data
  from the archive. Supports the earlier flat local-test installation, copying
  saves before removing its known application files and retaining unknown files.
- Creates `~/.local/bin/black-beacon` as a wrapper that invokes the packaged
  `black-beacon.sh` at its actual location (a direct symlink would break that
  launcher's dirname-based binary lookup). Forwards all arguments.
- Uses a shared flock during play and an exclusive lock for install/uninstall;
  updating while running through the managed launcher fails clearly. Close games
  launched directly from the package before updating as well.
- Creates a quoted `.desktop` entry, including paths with spaces and special
  characters. Warns when `~/.local/bin` is missing from PATH without editing shell
  startup files. Prints the release tag after installation/update.
- Unchanged versions skip the large archive download while refreshing launchers.
  Temporary downloads/staging are cleaned on errors, success, SIGINT/SIGTERM/HUP.
  An uncatchable SIGKILL/power loss can leave an inactive stage; the active package
  and persistent save trees are separate. Atomic filesystem changes do not imply
  a guarantee against hardware failure.

`uninstall-black-beacon.sh` removes managed application files and shortcuts while
keeping saves/config by default. `--remove-saves` explicitly removes only managed
userdata. Native Unreal directories outside this install root are never removed.
Unknown arguments, unrecognized installations and active game locks fail safely.

## Evidence

- `bash -n install.sh uninstall-black-beacon.sh`: passed.
- `shellcheck install.sh uninstall-black-beacon.sh`: passed.
- `desktop-file-validate`: passed in the special-character HOME test.
- `./Tools/validate.sh`: passed existing logic and seven architectural checks.
- **13/13 distribution integration tests passed**, including real r2 archive
  verification/extraction/uninstall without rebuilding, repackaging or launching
  the game. Synthetic release responses replace only HTTP transport; production
  extraction, checksumming, shell entry point, filesystem switching and launchers
  are exercised. Real GitHub download/publishing has not been tested yet.

Reproduce the full distribution tests:

```bash
BB_TEST_REAL_ARCHIVE="$PWD/dist/BlackBeacon-linux-x86_64-test1-r2.tar.zst" \
  python3 Tests/distribution/test_install.py
```

Without that environment variable, the real-archive test is skipped; all fixture
failure/update tests still run. Evidence logs: `Saved/DistributionValidation/`
(ignored). Archive size measured directly: 856,936,099 bytes; checksum file: 108.

## Publication state

[Release notes](ReleaseNotes/linux-test1-r2.md) and
[machine-readable manifest](ReleaseNotes/linux-test1-r2.json) are prepared locally.
Confirmed public repository: `daan-net/black-beacon`, default branch `main`,
prerelease/tag `linux-test1-r2`. The owner authorized full history publication,
existing branch/tag preservation and anonymous installation verification.
The remote repository is initially empty and its public metadata was verified.
Origin is configured to `https://github.com/daan-net/black-beacon.git`.

CLI authentication was absent at the start of publication. The owner was asked
to run `gh auth login`; no credentials were requested in chat. Release upload and
public end-to-end verification remain pending until authenticated writes work.
No Actions/workflows are added or enabled.

GitHub API behavior: [official release API documentation](https://docs.github.com/en/rest/releases/releases).
The installer does not use stable-only `/releases/latest`, because the proposed
first package is a tester prerelease.
