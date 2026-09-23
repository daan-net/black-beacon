# BLACK BEACON — Current State

**Last updated:** 2026-09-23 · **Milestone:** M0.1 Phase B

## Verified on this machine

| Item | Status | Evidence |
|---|---|---|
| UE 5.8.2 Linux editor target | `TESTED` | `BlackBeaconEditor Linux Development` UHT, Clang compile, and link succeeded after Phase B changes. |
| Actual project editor startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -log -NoSplash` initialized, loaded Entry, and passed map check (0 errors, 0 warnings); it remained open for the planned 70-second smoke run. |
| Rendered game startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -game -log` initialized on the NVIDIA RTX 2060 Vulkan device and remained up for a 45-second smoke run without a fatal error. |
| Player walk, sprint, crouch, look inputs | `TESTED` | In-engine `BlackBeacon.M01.PlayerControls` succeeded with injected W, Shift, C, MouseX, and MouseY events; it checked movement, sprint/crouch state, capsule height, eye height, and bounded camera yaw/pitch response. |
| First-person feel follow-up | `TESTED` | Legacy UE mouse scaling is disabled; yaw response is now 0.35 degrees per count, pitch response is 0.18, and the vertical sign was corrected after direct play feedback. Walk/sprint remain 400/700 cm/s. PlayerControls passed with the 152 cm standing eye height, supported crouch, stronger braking, and higher ground friction. Final feel requires a new human playtest. |
| Greybox stair lighting | `TESTED` | Three warm unshadowed point lights, one per tower floor, are built and exercised by the rendered M0.1 suite. They improve route visibility with no new collision or gameplay dependency; final brightness remains a human playtest judgement. |
| Nine-step objective, interaction, power, beam, reveal loop | `TESTED` | In-engine `BlackBeacon.M01.GameplayFlow` succeeded in `-game` with NullRHI and with Vulkan on the RTX 2060. It checks actual pawn overlaps, trace-based generator and lantern interactions, prompt changes, four-second spin-up and power loss, manual beam aim, the persistent anomaly reveal, and objective completion. |
| Physical greybox stair climb | `TESTED` | The stair has 84 steps with an 18.6 cm rise, 80 cm run, and 240 cm radial depth. A central column closes the inner fall; thinner 95 cm outer guards leave each revolution's entry tread open. The lantern landing now continues beyond the final tread instead of occupying the capsule's head space. StairTraversal reached lantern height and remained grounded in the complete rendered M0.1 regression; human collision review remains necessary. |
| Configured rain fog and spotlight settings | `TESTED` | GameplayFlow checks fog, spotlight scattering, visibility, and gameplay beam direction. The analytic shaft now uses wider silhouette falloff, low-frequency world-space mist variation, and a warmer linear light colour based on the local `Manual_Visuals` references. Spotlight intensity was reduced from 1500 to 1200 lumens to control impact clipping. Five 1920×1080 Vulkan captures show a clear B_Air versus B_AirOff difference. Final human review of the moving beam remains open. |
| M0.1 in-engine regression | `TESTED` | The full rendered Vulkan `BlackBeacon.M01` suite passed 3/3 on 2026-09-23 after the reference-driven beam pass: GameplayFlow, PlayerControls, StairTraversal. It regenerated all five beam captures. A new 3000-frame UE CSV capture on RTX 2060 recorded 5.78 ms median frame time and 5.35 ms median GPU time after the first 500 frames; the final 500-frame segment measured 5.81 ms median GPU time. The run covers startup and early gameplay rather than an isolated ON/OFF benchmark. |
| Plain C++ logic and project sanity | `TESTED` | `./Tools/validate.sh`: project checks green; 45/45 logic tests pass. |
| Procedural world geometry | `PLACEHOLDER` | Runtime greybox is correctly positioned and its objective volumes work; it remains a development stand-in for an authored level. |
| Save serialization and authored visuals | `PLANNED` | Later milestones; not part of this bring-up. |

## Remaining M0.1 acceptance work

The objective integration test teleports the pawn into the lantern room to exercise the climb volume; the separate stair traversal test proves the character can walk the full stair route. The 2026-09-23 rendered GameplayFlow run passed and regenerated A_Exterior, B_Air, B_AirOff, C_Impact, and D_Reveal. Beam ON/OFF, aim alignment, reveal function, and reasonable aggregate RTX 2060 performance are verified. The shaft uses opacity 0.04, softened analytic edges, subtle world-space variation, and a 1200-lumen warm spotlight. The procedural greybox remains much darker and simpler than the `Manual_Visuals` target, which belongs to M0.2/M0.3. M0.1 remains open only for direct human review of the moving beam. Do not start M1.

The UE 5.8.2 installed build is `/home/a1/WORK/_TOOLS/UE_5.8.2` (`~/UnrealEngine` points to it). Engine, build, logs, screenshots, and temporary test output are excluded from git.
