#!/usr/bin/env bash
# ============================================================
# BLACK BEACON - local validation (no Unreal Engine required)
# ============================================================
# Runs every check that can run without the engine:
#   1. Project file sanity (.uproject JSON, config ini sections)
#   2. Builds and runs the logic-layer unit tests
#
# Exit code 0 = everything that CAN be validated here is green.

set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$(pwd)"
FAILED=0

step() { printf '\n== %s ==\n' "$*"; }
fail() { printf 'FAIL: %s\n' "$*"; FAILED=1; }

# ---------------------------------------------------------
step "1/2  Project file sanity"

python3 - "$ROOT" <<'PY'
import json, sys
root = sys.argv[1]

# .uproject must be valid JSON with the expected module entry
with open(f"{root}/BlackBeacon.uproject") as f:
    proj = json.load(f)
modules = {m["Name"]: m for m in proj.get("Modules", [])}
assert "BlackBeacon" in modules, "missing module entry"
assert modules["BlackBeacon"]["Type"] == "Runtime", "module must be Runtime"
print("  .uproject  : valid JSON, module entry OK")

# Every [/Script/...] section referenced in ini must mention a class
# that appears somewhere under Source/ (cheap static cross-check).
import re, pathlib
source = "".join(p.read_text(errors="ignore") for p in pathlib.Path(f"{root}/Source").rglob("*.*"))
for ini in pathlib.Path(f"{root}/Config").glob("Default*.ini"):
    text = ini.read_text(errors="ignore")
    for m in re.finditer(r"\[/Script/(\w+)\.(\w+)\]", text):
        module, cls = m.group(1), m.group(2)
        if module == "BlackBeacon":
            assert cls in source or cls in open(f"{root}/BlackBeacon.uproject").read(), \
                f"{ini.name}: class {cls} not found in Source/ (typo?)"
            print(f"  {ini.name:<20}: {cls} referenced OK")
PY

# ---------------------------------------------------------
step "2/2  Logic-layer unit tests"

if [ -d "$ROOT/Tests/build" ]; then
    cmake -S "$ROOT/Tests" -B "$ROOT/Tests/build" -G Ninja > /dev/null
else
    cmake -S "$ROOT/Tests" -B "$ROOT/Tests/build" -G Ninja > /dev/null
fi
cmake --build "$ROOT/Tests/build" > /dev/null
"$ROOT/Tests/build/bb_logic_tests" > /dev/null
echo "  bb_logic_tests : ALL PASS"

# ---------------------------------------------------------
step "Summary"
if [ "$FAILED" -eq 0 ]; then
    echo "VALIDATION OK - project files sane, logic tests pass."
else
    echo "VALIDATION FAILED - see messages above."
    exit 1
fi