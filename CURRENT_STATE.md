# BLACK BEACON — Current State

**Last updated:** 2026-09-23 · **Milestone:** M0.1 Phase B

## Verified on this machine

| Item | Status | Evidence |
|---|---|---|
| UE 5.8.2 Linux editor target | `TESTED` | `BlackBeaconEditor Linux Development` UHT, Clang compile, and link succeeded after Phase B changes. |
| Actual project editor startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -log -NoSplash` initialized, loaded Entry, and passed map check (0 errors, 0 warnings); it remained open for the planned 70-second smoke run. |
| Rendered game startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -game -log` initialized on the NVIDIA RTX 2060 Vulkan device and remained up for a 45-second smoke run without a fatal error. |
| Player walk, sprint, crouch, look inputs | `TESTED` | In-engine `BlackBeacon.M01.PlayerControls` succeeded with injected W, Shift, C, MouseX, and MouseY events; it checked movement, sprint/crouch state, capsule height, eye height, and bounded camera yaw/pitch response. |
| Generator interaction usability | `TESTED` | The generator greybox was raised from a partly buried 110 cm top to a 180 cm top, and the camera interaction query now uses a configurable 24 cm sphere sweep. GameplayFlow focuses and starts it while aiming naturally at camera height; the first blocking surface still prevents interaction through walls. |
| First-person feel follow-up | `TESTED` | Legacy UE mouse scaling is disabled; yaw response is now 0.35 degrees per count, pitch response is 0.18, and the vertical sign was corrected after direct play feedback. Walk/sprint remain 400/700 cm/s. PlayerControls passed with the 152 cm standing eye height, supported crouch, stronger braking, and higher ground friction. Final feel requires a new human playtest. |
| Greybox stair lighting | `TESTED` | Three warm unshadowed point lights, one per tower floor, are built and exercised by the rendered M0.1 suite. They improve route visibility with no new collision or gameplay dependency; final brightness remains a human playtest judgement. |
| Nine-step objective, interaction, power, beam, reveal loop | `TESTED` | In-engine `BlackBeacon.M01.GameplayFlow` succeeded in `-game` with NullRHI and with Vulkan on the RTX 2060. It checks actual pawn overlaps, sweep-based generator and lantern interactions, prompt changes, four-second spin-up and power loss, manual beam aim, the persistent anomaly reveal, and objective completion. |
| Physical greybox stair climb | `TESTED` | Review of `Debug_Manual/Screencast From 2026-09-23 08-58-12.mp4` exposed an awkward doorway gap and treads intersecting the central column. The 84-step route now has a doorway landing, 220 cm path radius, 180 cm radial tread depth, and a smaller 70 cm core radius, leaving 60 cm of clear inner separation. StairTraversal reached lantern height and remained grounded in the complete rendered M0.1 regression; the revised entrance and transitions still need direct human feel review. |
| Configured rain fog and spotlight settings | `TESTED` | GameplayFlow checks fog, spotlight scattering, visibility, and gameplay beam direction. The analytic shaft uses wider silhouette falloff, low-frequency world-space mist variation, and a warmer linear light colour based on the local `Manual_Visuals` references. Spotlight intensity is 1200 lumens. Five rendered captures show a clear B_Air versus B_AirOff difference, and direct review of the supplied moving video confirmed that the beam starts, reads, and rotates correctly. |
| M0.1 in-engine regression | `TESTED` | After the video-driven stair correction, UE 5.8.2 `BlackBeaconEditor` built and the full rendered Vulkan `BlackBeacon.M01` suite passed 3/3 on the RTX 2060: GameplayFlow, PlayerControls, and StairTraversal, with zero warnings or errors. `./Tools/validate.sh` also passed all 45 logic checks. |
| Plain C++ logic and project sanity | `TESTED` | `./Tools/validate.sh`: project checks green; 45/45 logic tests pass. |
| Procedural world geometry | `PLACEHOLDER` | Runtime greybox is correctly positioned and its objective volumes work; it remains a development stand-in for an authored level. |
| Save serialization and authored visuals | `PLANNED` | Later milestones; not part of this bring-up. |

## Remaining M0.1 acceptance work

The objective integration test teleports the pawn into the lantern room to exercise the climb volume; the separate stair traversal test proves the character can walk the full route. Direct review of the supplied video accepts beam startup, readability, and rotation for M0.1. The same video exposed the remaining stair entrance and steering problem, so the geometry was corrected and passed automated physical traversal. M0.1 remains open for one direct human traversal of the revised entrance and floor transitions. Do not start M1.

The UE 5.8.2 installed build is `/home/a1/WORK/_TOOLS/UE_5.8.2` (`~/UnrealEngine` points to it). Engine, build, logs, screenshots, and temporary test output are excluded from git.
