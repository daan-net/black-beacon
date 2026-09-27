# V0.5.1 playtest candidate finalization

Safety baseline: `a036ba4`, branch `work/v051-playtest-candidate`.
Recovery on 2026-09-27 found a clean tree at that commit. The failed external
wooden-door relocation and temporary diagnostic instrumentation were absent;
the door source retained Y=-113. Existing diagnostic reports were preserved.

## Doorway root cause and correction

The actual blocker was `Actor_8 / Mesh`, a `StaticMeshComponent` tagged
`BB_StairGuard`: the first ground-flight guard, at actor location
(241.782, -55.185, 75.857), yaw -12.857 degrees. Its 6 × 80 × 95 cm collision
box projected 40 cm beyond the first visible railing post into the entry.

Full-scene capsule sweeps (radius 38, half-height 88, ignoring only the player)
blocked on this guard for -3 and 0 degrees at BOTH Z=112 and Z=150. The +3-degree
sweep cleared it. The old passing static checks excluded every actor except the
lighthouse, which excluded the stair guard. Height alone did not explain the
mismatch.

During the -3-degree dynamic approach, the first contact was:

- Capsule hit location: (291.575, -15.282, 108.490).
- Impact point: (253.608, -16.856, 91.755).
- Capsule contact normal: (0.999, 0.041, 0).
- Mesh face impact normal: (0.975, -0.223, 0).
- Start penetrating: false; penetration depth: 0.
- Step-up was eligible; maximum step height remained 45 cm. The raised movement
  attempt hit the same corner at capsule Z=151.571, then returned to the landing.
  The 95 cm guard could not be stepped over.
- Later capsule contact normal: (0.975, 0.223, 0), with velocity
  (-10.824, +47.424, 0), approximately perpendicular to that normal. This explains
  the observed loss of forward speed and lateral slide.
- Supporting floor was `Actor_3 / Mesh`, tagged `BB_StairEntryLanding`, top
  Z=18.571 and upward normal. It was not the horizontal obstruction.

The V0.5.1 collision adapter now trims only the first guard's entrance-facing
half: tangential length 80→40 cm and center shift 20 cm toward the next post.
Its end aligns with the existing first railing post. All upper guards, stair
risers, tower perimeter, door geometry and gameplay logic are unchanged.

Temporary hit instrumentation was removed before the final build. Persistent
regression coverage now includes full-scene sweeps at Z=112 and Z=150 and the
original three dynamic W approaches. A level sweep may hit a tagged stair tread
within the character's 45 cm step limit; it may not ignore a guard or wall. After
the fix, dynamic entry reached X=252.252, 251.387, and 240.055 for -3°, 0°, +3°
in the unchanged 0.6 seconds. The 0.6-second limit and X<280 criterion
are unchanged.

Diagnostic evidence: `Saved/PlaytestV051/Finalization/DoorCollision.log` and
`TemporaryDiagnostics.patch` (ignored evidence, not runtime source).

## Validation

- Unreal 5.8.2 Linux Development editor build: succeeded.
- `./Tools/validate.sh`: passed logic and architectural source checks.
- `BlackBeacon.V051.PlaytestLoop`: **TESTED — Success**, 40.05 seconds, exit 0.
- `BlackBeacon.M01.GameplayFlow`: **TESTED — Success**, 9.59 seconds, exit 0.

The complete loop was also exercised in Vulkan during finalization; that run
passed all gameplay checks and exposed only the new static probe's incorrect
rejection of a reachable tread. After correcting that assertion, the final
targeted test and M01 regression ran with NullRHI to avoid duplicate renders.

Final evidence is retained under `Saved/PlaytestV051/Finalization/`.
Automation validates the technical candidate; owner manual UX acceptance remains
separate, particularly stair comfort with a physical mouse and search readability.

## Manual controls

WASD or arrow keys move; mouse looks; Left Shift sprints; C toggles crouch.
E interacts with the generator. At the lantern console, E starts the powered
beacon; press E again to take control. Mouse X changes yaw and mouse Y changes
pitch, limited to -20° through +8°. Sweep the sea and hold on the wreck until it
reveals and discovery is acknowledged. E releases from any search direction;
normal camera and movement return. Face the console and press E to reacquire.
F5 saves and F9 loads.
