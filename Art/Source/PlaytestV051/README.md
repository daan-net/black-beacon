# V0.5.1 access corrections

`generate_access.py` exports only the existing gallery and lantern room with the
service-door opening extended to standing headroom. The console moves from the
opening to the supported southwest floor, facing into the room. The original
V0.4 and V0.5 generators retain their default output when called without the
`playtest_access` option.

Run `python3 Art/Source/PlaytestV051/generate_access.py`, then run
`import_access.py` in Unreal's Python commandlet. The import preserves the
existing material assignments and disables glass shadow casting. It does not
import the gallery collision deck or regenerate any material.

Runtime simple collision follows the tapered masonry, lantern perimeter and
outer gallery rail. The +X ground doorway and +X gallery service door remain
open. Existing stair and deck collision stays separate from visual meshes.
