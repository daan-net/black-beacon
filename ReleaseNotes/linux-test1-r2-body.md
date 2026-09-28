Black Beacon Linux x86_64 playtest, **test1-r2**.

This is the previously validated packaged build, including the lighthouse,
generator, full traversal, beacon yaw/pitch search and shipwreck reveal, with
the r2 asset-cooking and NVIDIA Vulkan pipeline-cache fixes. No rebuild or
repackaging was performed for this release.

Install or update:

```bash
curl -fsSL https://raw.githubusercontent.com/daan-net/black-beacon/main/install.sh | bash
```

Run `black-beacon` or use the application-menu entry. Close the game before
updating. Requires Linux x86_64, a working Vulkan driver, Bash, curl, Python 3,
zstd and flock. No Unreal Editor or compiler is required.

The installer verifies SHA-256 before extraction, switches application versions
atomically, and preserves saves/config. Repeating an unchanged install skips
the archive download. See the repository README for uninstall instructions.

Archive SHA-256:

```text
583419c562d3e07b7b2fdfd42fb136a885896020a4348649c91614f7e2dc7df4
```

Assets: `BlackBeacon-linux-x86_64-test1-r2.tar.zst` (856,936,099 bytes) and its
matching `.sha256`. Package source: gameplay/art `b232ff1`, packaging fixes
`a4b63a1`. This prerelease still needs tester feedback on mouse comfort and
hardware compatibility. It is a Development build; the existing archive includes
debug artifacts and developer Saved logs/screenshots, which the installer excludes
from tester save/config directories.
