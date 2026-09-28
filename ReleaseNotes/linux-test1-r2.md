# Black Beacon — Linux playtest test1-r2

Published and publicly verified: [linux-test1-r2](https://github.com/daan-net/black-beacon/releases/tag/linux-test1-r2).
Release tag commit: `121cd4c1a857f718e2a08414ae988a230e7ac46a`.

## Confirmed publication target

- Repository: `daan-net/black-beacon` — verified public repository.
- Default branch: `main` — verified repository default branch.
- Visibility: public is required for the documented anonymous installation.
  Public visibility and full repository-history publication were explicitly approved.
- Release/tag: `linux-test1-r2`.
- Title: `Black Beacon Linux playtest test1-r2`.
- Mark as **prerelease**. The installer deliberately includes published tester
  prereleases instead of using GitHub's stable-only `/releases/latest` endpoint.
- Installer URL: `https://raw.githubusercontent.com/daan-net/black-beacon/main/install.sh`.
- Preserve and push existing repository branches/history and checkpoint tags;
  fast-forward main to the validated distribution infrastructure. No force-push.

## Exact assets — upload existing files unchanged

| File | Bytes |
| --- | ---: |
| `dist/BlackBeacon-linux-x86_64-test1-r2.tar.zst` | 856936099 |
| `dist/BlackBeacon-linux-x86_64-test1-r2.tar.zst.sha256` | 108 |

```text
583419c562d3e07b7b2fdfd42fb136a885896020a4348649c91614f7e2dc7df4  BlackBeacon-linux-x86_64-test1-r2.tar.zst
```

The existing checksum was verified against the existing archive. No rebuild,
recompression or package modification was performed. The archive includes its
existing developer Saved logs/screenshots; the installer skips all packaged
Saved trees and retains tester-owned saves/config instead.

Archive provenance: gameplay/art checkpoint `b232ff1`, packaging fixes `a4b63a1`,
validation report at `cfb92f4`. This is the already validated Linux x86_64
Development package, not a newly built game. See `LINUX_PACKAGE_VALIDATION.md`.

## Release summary

Black Beacon Linux x86_64 playtest, test1-r2. Includes the validated lighthouse,
traversal, generator, beacon search and shipwreck reveal loop, with the r2
cooking and NVIDIA Vulkan pipeline-cache fixes.

Install/update:

```bash
curl -fsSL https://raw.githubusercontent.com/daan-net/black-beacon/main/install.sh | bash
```

Run `black-beacon`. Close the game before updating. Requires a Linux x86_64
system with a working Vulkan driver plus curl, Python 3, zstd and flock.
The installer verifies SHA-256 before installation and preserves saves/config.
This is a prerelease playtest build; physical-mouse UX and wider hardware
compatibility still need tester feedback.

## Publication authorization and verification

The owner confirmed `daan-net/black-beacon`, public visibility, default branch
`main`, prerelease/tag `linux-test1-r2`, full history push and the two existing
assets. No further publication approval is needed. Origin now uses the approved
HTTPS URL. Preserve all existing branches and tags; never force-push.

The draft was published after both uploaded asset sizes and GitHub SHA-256
digests matched the existing local inputs. All branches and previous tags were
preserved. Repository Actions is disabled; no CI, rebuild or repackaging.

The anonymous public installer passed from an isolated temporary HOME, including
real download, checksum, extraction, desktop entry, actual packaged headless
command startup (exit 0), and repeat installation with saved data retained and
no duplicate payload. See [verification evidence](../DISTRIBUTION.md).
