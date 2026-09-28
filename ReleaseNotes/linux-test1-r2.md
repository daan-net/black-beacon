# Black Beacon — Linux playtest test1-r2

Local release preparation only; **not uploaded or published**.

## Proposed publication

- Repository: `daan-net/black-beacon` — proposal, not yet confirmed/created.
- Default branch: `main` — proposal, not verified for a destination repository.
- Visibility: public is required for the documented anonymous installation.
  No repository visibility has been changed; actual destination is unconfirmed.
- Release/tag: `linux-test1-r2`.
- Title: `Black Beacon Linux playtest test1-r2`.
- Mark as **prerelease**. The installer deliberately includes published tester
  prereleases instead of using GitHub's stable-only `/releases/latest` endpoint.
- Installer URL: `https://raw.githubusercontent.com/daan-net/black-beacon/main/install.sh`.
- Publication should contain only approved distribution files if the source
  repository is to remain private. This checkout has no Git remote configured.

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

## Release body ready for review

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

## Publication gate

Before any remote write, confirm owner/repository, default branch and public
visibility, and show the final command/asset list to the owner as requested.
The proposal above is not evidence that a public repository already exists.
No tag was created or moved locally or remotely for this release.

After confirmation, publish the installer/docs to the approved default branch,
create the release initially as a draft prerelease, upload precisely the two
assets above, verify their size/checksum and only then publish the draft.
Use the prepared body via `gh --body-file` or equivalent structured API fields.
No GitHub Actions, CI build, engine rebuild or automatic deployment is involved.

GitHub CLI is installed but currently unauthenticated. The connected GitHub app
identifies `daan-net`; no Black Beacon repository was found among its accessible
owned repositories. Release upload capability/authentication must be available
before the existing 817 MiB archive can be uploaded.
