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
| Configured rain fog and spotlight settings | `TESTED` | GameplayFlow checks rain fog density, volumetric fog enabled on the fog component, spotlight scattering, and light visibility on power changes. A rendered screenshot is written under ignored `Saved/Screenshots/LinuxEditor/`; the capture is too obscured to establish that the beam reads clearly in play. |
| Plain C++ logic and project sanity | `TESTED` | `./Tools/validate.sh`: project checks green; 45/45 logic tests pass. |
| Procedural world geometry | `PLACEHOLDER` | Runtime greybox is correctly positioned and its objective volumes work; it remains a development stand-in for an authored level. |
| Save serialization and authored visuals | `PLANNED` | Later milestones; not part of this bring-up. |

## Remaining M0.1 acceptance work

The integration test teleports the pawn into the lantern room to exercise the climb volume; it does not prove a player can physically walk the entire stair route. The rendered capture is obscured by fog, so beam readability is still unverified. Manual editor Play traversal and a clear side-view beam check are needed before claiming M0.1 fully complete. Do not start M1.

The UE 5.8.2 installed build is `/home/a1/WORK/_TOOLS/UE_5.8.2` (`~/UnrealEngine` points to it). Engine, build, logs, screenshots, and temporary test output are excluded from git.
