# BLACK BEACON — Current State

**Last updated:** 2026-09-22 · **Milestone:** M0.1 Phase B

## Verified on this machine

| Item | Status | Evidence |
|---|---|---|
| UE 5.8.2 Linux editor target | `TESTED` | `BlackBeaconEditor Linux Development` UHT, Clang compile, and link succeeded after Phase B changes. |
| Actual project editor startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -log -NoSplash` initialized, loaded Entry, and passed map check (0 errors, 0 warnings); it remained open for the planned 70-second smoke run. |
| Rendered game startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -game -log` initialized on the NVIDIA RTX 2060 Vulkan device and remained up for a 45-second smoke run without a fatal error. |
| Player walk, sprint, crouch, look inputs | `TESTED` | In-engine `BlackBeacon.M01.PlayerControls` automation test succeeded with injected W, Shift, C, and MouseX events; it checked pawn movement, sprint/crouch state, capsule height, and camera yaw. |
| Nine-step objective, interaction, power, beam, reveal loop | `TESTED` | In-engine `BlackBeacon.M01.GameplayFlow` succeeded in `-game` with NullRHI and with Vulkan on the RTX 2060. It checks actual pawn overlaps, trace-based generator and lantern interactions, prompt changes, four-second spin-up and power loss, manual beam aim, the persistent anomaly reveal, and objective completion. |
| Physical greybox stair climb | `TESTED` | In-engine `BlackBeacon.M01.StairTraversal` moved the actual character across all 66 stair steps to lantern height and checked grounded movement. It passed alongside the other M0.1 tests after replacing the obstructing full lantern floor with a compact landing. |
| Configured rain fog and spotlight settings | `TESTED` | GameplayFlow checks fog, spotlight scattering, visibility, and gameplay beam direction. Five 1920×1080 Vulkan captures under ignored `Saved/Screenshots/LinuxEditor/` show a visible B_Air versus B_AirOff shaft, softer beam edges, and reduced C_Impact/D_Reveal clipping. A dim moon light now shows the lighthouse and terrain in A_Exterior. Final human review of the moving beam remains open. |
| M0.1 in-engine regression | `TESTED` | The full rendered Vulkan `BlackBeacon.M01` suite passed after the lens and mast changes: GameplayFlow, PlayerControls, StairTraversal. A UE CSV capture on RTX 2060 recorded median frame time 5.59 ms and median GPU time 5.32 ms over 3000 frames after the first 500; this covers startup and early gameplay. A longer 6000-frame capture stopped just as GameplayFlow started, so there is no valid isolated beam-on/off cost comparison yet. |
| Plain C++ logic and project sanity | `TESTED` | `./Tools/validate.sh`: project checks green; 45/45 logic tests pass. |
| Procedural world geometry | `PLACEHOLDER` | Runtime greybox is correctly positioned and its objective volumes work; it remains a development stand-in for an authored level. |
| Save serialization and authored visuals | `PLANNED` | Later milestones; not part of this bring-up. |

## Remaining M0.1 acceptance work

The objective integration test teleports the pawn into the lantern room to exercise the climb volume; the separate stair traversal test proves the character can walk the full stair route. The 2026-09-22 rendered GameplayFlow run passed and regenerated A_Exterior, B_Air, B_AirOff, C_Impact, and D_Reveal. Beam ON/OFF, aim alignment, and reveal function are verified. The analytic shaft fades at its silhouette, with opacity 0.04 and a 1500-lumen spotlight; low-cost moonlight, a powered source glow, and a small emissive lens improve the exterior view. A non-colliding greybox mast grounds the lens on the tower until authored lantern housing exists. M0.1 remains open for direct moving-beam visual review and a beam-on/off performance comparison. Do not start M1.

The UE 5.8.2 installed build is `/home/a1/WORK/_TOOLS/UE_5.8.2` (`~/UnrealEngine` points to it). Engine, build, logs, screenshots, and temporary test output are excluded from git.
