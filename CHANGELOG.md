# BLACK BEACON — Change Log

Format: `date — milestone — summary`. Version reflects the vertical slice
(`0.x`); no released build exists yet. A change that alters an existing
user-visible behaviour is called out as **user-visible**.

Status vocabulary used here (same as AGENTS.md): `IMPLEMENTED` (built in the
intended context), `TESTED` (built + exercised with evidence), `PLACEHOLDER`
(scaffolding to be replaced), `PLANNED` (paper only). Anything written-but-not-
compiled is marked **written, not built** and is *not* `IMPLEMENTED`.

---

## 2026-09-23 — M0.2 — shipwreck coast silhouette and atmosphere

- **User-visible:** Replaced the geometric basic shapes of the shipwreck coast with a denser, procedural array of non-uniformly scaled and rotated cubic structures. This creates a more natural, jagged rocky silhouette framing the lighthouse.
- **User-visible:** Added a physical SkyAtmosphere and SkyLight component to the weather controller, and tied the moon's directional light into the atmosphere. The opening frame now features a cohesive dark sky and moody nocturnal atmosphere instead of an empty black void.
- UE 5.8.2 `BlackBeaconEditor` built successfully. The Vulkan M0.1 suite passed, regenerating the `BlackBeacon_M02_Opening.png` screenshot. `./Tools/validate.sh` remains green with 45/45 logic checks. The opening presentation is now much more atmospheric.

## 2026-09-23 — M0.2 — shipwreck opening foundation

- **User-visible:** the slice now fades in from black on the island shore with the player's view aimed at the distant, unpowered lighthouse. The ARRIVE objective describes waking on the shore, and the design documents record the shipwreck-survivor premise plus the asylum and illegal-experiment thread as wider story direction.
- Added a placed coast presentation actor with dark wet rock forms, a restrained route toward the lighthouse, and sparse wreckage. Its geometry has no collision, so it cannot alter the accepted player route or objective flow. Added a parameterized coast material instead of relying on the fixed white Engine shape material.
- Extended GameplayFlow with a real post-fade opening capture and checks that the placed coast actor loads at the intended origin. UE 5.8.2 `BlackBeaconEditor` built successfully; the full rendered Vulkan M0.1 suite passed 3/3 after placement, and GameplayFlow passed again after the material correction. The opening composition is `PLACEHOLDER`: the framing reads, but the basic coast forms and empty sky still require M0.2 presentation work.

## 2026-09-23 — M0.2 — persistent vertical-slice map

- Added `/Game/BlackBeacon/Maps/L_BlackBeacon_M02` as the authored game and editor startup map, containing the verified M0.1 actors and PlayerStart. Disabled runtime procedural bootstrap by default; the C++ builder remains available as development tooling.
- Made dynamically created builder components persistent instance components so generated actors serialize correctly into an Unreal map.
- Made objective overlap registration idempotent. The first authored-map run exposed a duplicate serialized delegate binding; `AddUniqueDynamic` removes the startup ensure without changing trigger behavior.
- UE 5.8.2 `BlackBeaconEditor` built successfully. The complete rendered Vulkan suite passed 3/3 from the saved map with zero warnings or errors on the RTX 2060. The map is `TESTED`; its geometry remains `PLACEHOLDER` pending M0.2 polish.

## 2026-09-23 — M0.1 Phase B — reference-driven beam refinement

- **Milestone complete:** direct play review accepted the moving beam, generator flow, player controls, revised stair entrance, and transitions. M0.1 now has a real UE 5.8.2 build, stable project startup, 3/3 rendered Vulkan automation tests, and 45/45 engine-independent checks. M0.2 is next; no broad M1 work has started.

- **User-visible:** adjusted the shaft against the local `Manual_Visuals` references: warmer linear light colour, broader silhouette falloff, and subtle low-frequency world-space mist variation. The material remains one texture-free analytic cone synchronized with the gameplay query.
- **User-visible:** reduced the spotlight from 1500 to 1200 lumens to soften impact clipping while retaining a clear B_Air/B_AirOff difference. BeamReveal geometry and state logic are unchanged.
- UE 5.8.2 `BlackBeaconEditor` built successfully. The rendered Vulkan `BlackBeacon.M01` suite passed 3/3 and regenerated all five 1920×1080 captures. A 3000-frame RTX 2060 CSV capture measured 5.78 ms median frame time and 5.35 ms median GPU time after the first 500 frames; this is aggregate startup/early-game evidence rather than an isolated ON/OFF comparison. `./Tools/validate.sh` remains required before checkpointing. M0.1 awaits direct moving-beam visual review.
- **User-visible:** fixed the generator prompt failing to appear at a natural viewing angle. The greybox generator was partly below the floor and ended below eye height; it now stands from floor level to 180 cm, and interaction uses a configurable 24 cm sphere sweep instead of a zero-width ray. The sweep still stops at the first blocking surface. GameplayFlow now verifies focus and startup while aiming horizontally at camera height.
- Direct review of the supplied gameplay video confirmed that the beam starts, remains readable, and rotates correctly. The review also showed the real cause of the awkward stair steering: 240 cm treads extended 10 cm into the 90 cm-radius central column, while the doorway ended directly at the narrow first tread.
- **User-visible:** moved the stair path radius from 200 to 220 cm, reduced radial tread depth from 240 to 180 cm, reduced the core radius from 90 to 70 cm, and added a level doorway landing. The resulting 60 cm separation removes intersecting geometry and gives the 38 cm-radius player capsule room to turn. The lantern landing follows the new route.
- UE 5.8.2 `BlackBeaconEditor` built successfully. The complete Vulkan `BlackBeacon.M01` suite passed 3/3 with zero warnings or errors on the RTX 2060, including physical traversal over all 84 steps and new assertions for the entrance landing and path radius. `./Tools/validate.sh` passed with 45/45 logic checks. Human feel review of the revised stair remains the M0.1 gate.

## 2026-09-22 — M0.1 Phase B — first-person control and stair readability

- **User-visible:** disabled Unreal's legacy 2.5× controller mouse scales and added explicit horizontal/vertical Enhanced Input sensitivities. Vertical mouse input is now intentionally inverted at the controller boundary, and manual beam aim uses the same lower sensitivity scale.
- **User-visible:** moved the first-person viewpoint from the capsule centre to a 152 cm standing eye height, switched crouch to the engine's supported crouch path, and tuned walk/sprint/crouch speeds, braking, and ground friction for the narrow helical stair.
- **User-visible:** added one warm, unshadowed fill light per greybox tower floor. This improves stair visibility without changing geometry, collision, objectives, or the lighthouse beam.
- Extended `BlackBeacon.M01.PlayerControls` to check human-scale eye height and bounded mouse yaw/pitch response. The isolated PlayerControls, GameplayFlow, and StairTraversal tests passed; the complete Vulkan `BlackBeacon.M01` suite then passed 3/3 and regenerated the five beam captures. `BlackBeaconEditor` built without warnings and `./Tools/validate.sh` remained green with 45/45 logic checks.
- **User-visible:** raised explicit mouse yaw/pitch response to 0.22/0.18 degrees per count and walk/sprint speed to 400/700 cm/s after the first direct control review found the prior tuning too slow.
- **User-visible:** rebuilt each stair revolution from 22 tall, incorrectly oriented treads to 28 treads with an 18.6 cm rise, 80 cm run, and 240 cm radial depth. Added a central collision column and 95 cm outer guards so the player cannot slip into either open edge during the climb. `BlackBeacon.M01.StairTraversal` now covers all 84 steps.
- Rebuilt `BlackBeaconEditor` with UE 5.8.2. The complete rendered Vulkan `BlackBeacon.M01` suite passed 3/3 after the new stair geometry and control tuning, and all five beam screenshots were regenerated. `./Tools/validate.sh` remained green with 45/45 logic checks. Direct control feel and stair proportions still require human review.
- **User-visible:** corrected the vertical mouse direction reported as inverted and increased horizontal look response from 0.22 to 0.35 degrees per count so the helical climb requires less mouse travel.
- **User-visible:** opened the first outer guard at each stair revolution, halved guard thickness, and moved the lantern landing beyond the final tread. This removes the collision squeeze at stair entries and the low overhead obstruction near the top. The UE 5.8.2 build and complete rendered M0.1 suite passed 3/3 after these changes; human traversal feel remains the acceptance check.

## 2026-09-22 — M0.1 Phase B — beam readability pass in progress

- **User-visible:** connected the analytic additive shaft material to the lighthouse visual mesh and synchronized its world-space origin, direction, length, and cone angle with the gameplay beam. Kept BeamReveal queries and rotation behavior unchanged.
- **User-visible:** reduced visual shaft opacity to 0.025 and spotlight intensity to 1500 lumens. The rendered B_Air/B_AirOff pair shows reliable ON/OFF visibility; C_Impact and D_Reveal have substantially less clipping. The beam still shows a crisp geometric edge and A_Exterior remains too dark, so visual acceptance and M0.1 closure are pending.
- UE 5.8.2 `BlackBeaconEditor Linux Development` built; rendered Vulkan `BlackBeacon.M01.GameplayFlow` passed and produced all five 1920×1080 captures on RTX 2060. `./Tools/validate.sh` passed, including the 45 logic checks. No M1 work was started.
- **User-visible:** softened the shaft material by fading its silhouette according to view angle and optical thickness; set final opacity to 0.04. Added a dim movable moon light to the weather controller and a powered source glow to the beam component so the exterior tower and beam origin remain legible in the greybox night scene.
- Rebuilt `BlackBeaconEditor`; the complete rendered `BlackBeacon.M01` suite passed (GameplayFlow, PlayerControls, StairTraversal). The five captures were regenerated. An RTX 2060 CSV capture measured 5.59 ms median frame time and 5.32 ms median GPU time over 3000 frames after the first 500, but was not an isolated beam-on/off benchmark. Direct visual review of the moving beam remains the M0.1 gate.
- **User-visible:** added a small emissive lens at the beam query origin. It switches with beam power, making the source readable in the exterior capture. The lens has no collision and adds one unshadowed mesh draw; authored lantern housing is still future work.
- **User-visible:** grounded the source lens on the tower with a narrow non-colliding greybox mast. It changes only the exterior visual read and does not alter the beam query or stair collision.
- The full three-test rendered M0.1 suite passed again after the mast change. A longer CSV profiling attempt ended at the start of GameplayFlow and did not produce a usable beam-on/off comparison; the narrower earlier frame-time sample remains the available performance evidence.

## 2026-09-22 — M0.1 Phase B — runtime greybox bring-up

- Corrected the reflected GameMode config path. The real `-game` launch then
  exposed and fixed player-start recursion and a Slate prompt crash. The
  procedural builder now applies mesh and trigger transforms, uses the
  configured landing point, and leaves passage through the tower and annex.
- Wired Enhanced Input classes and proper W/A/S/D and mouse axis mappings;
  made generator and lighthouse controls traceable through the existing
  interaction component. Prompts now refresh after an interaction changes
  the focused actor's state.
- **User-visible:** objective prerequisites in `DefaultGame.ini` now enforce
  the nine-step order. Generator start completes before power is granted, and
  stopping it removes power and turns off the beam. Beam operation waits for
  the player's lantern interaction.
- **User-visible:** rain fog and spotlight scattering are enabled in the real
  renderer. The rain fog density and greybox beam intensity were adjusted
  during Vulkan capture checks; visual readability remains unverified because
  the captured view is obscured. `bVolumetricLight` now drives the spotlight.
- Added in-engine `BlackBeacon.M01.PlayerControls` and
  `BlackBeacon.M01.GameplayFlow` automation checks. Both passed on UE 5.8.2;
  the flow also passed with Vulkan on the NVIDIA RTX 2060. It exercises pawn
  overlaps, trace interactions, prompts, generator spin-up and power loss,
  manual beam aim, persistent reveal, and completion of all objectives.
- Final visual readability remains an open M0.1 acceptance check. No M1 work
  was started.

### Stair traversal follow-up

- Replaced the obstructing full lantern floor with a compact landing beside
  the control. Added `BlackBeacon.M01.StairTraversal`, which moved the actual
  character over all original 66 greybox steps to lantern height in the engine. All
  three M0.1 automation tests pass together. Beam readability remains open.


## 2026-09-21 — M0.1 Phase A — UE 5.8.2 bring-up

- Fixed all 11 errors from the first real Unreal compile, plus the subsequent
  UHT include-order and Slate binding errors: matching beam update return
  contracts, current `EAllowShrinking` API, current string conversion header,
  object pointer validity/constness, spotlight intensity units, and prompt text.
- `BlackBeaconEditor` built successfully with UE 5.8.2 on Linux. The actual
  project opened in Unreal Editor, reached engine initialization, passed map
  check (0 errors, 0 warnings), and remained running for a 70-second smoke run
  without a startup fatal error. This is `TESTED` startup evidence, not a Play
  test of gameplay.
- `./Tools/validate.sh` remains green, with 45/45 engine-independent checks
  passing. No gameplay behavior or tunable was changed in this checkpoint.


## 2026-09-21 — M0 — Initial foundation bootstrap

The repository as first established on `main`. No prior history. Unreal Engine
is **not installed** on this machine, so the UE C++ layer is written but not
compiled; every UE system below is **written, not built** until M0.1 pass.

Recorded in git as six initial commits, `bce6bc6` (docs base) → `1f2bcf5`
(status + handoff docs) — see `git log` for the full history.

### Added

- **Documentation set (10 root docs):** `README_FIRST`, `AGENTS`, `MASTER_VISION`,
  `GAME_DESIGN`, `TECHNICAL_ARCHITECTURE`, `VISUAL_DIRECTION`, `MILESTONES`,
  `CHANGELOG`, `CURRENT_STATE`, `CODEX_HANDOFF`. Scope anchored to vertical
  slice 0.1 (GAME_DESIGN §1).
- **UE5 C++ project scaffold:** `BlackBeacon.uproject` (valid JSON,
  `EngineAssociation` blank for drop-in engine choice), `Config/DefaultEngine|Game|
  Input|Editor.ini`, `Source/BlackBeacon.Target.cs`, `Source/BlackBeaconEditor.Target.cs`
  (`.Latest` engine versions), `Source/BlackBeacon/BlackBeacon.Build.cs` (module
  `BlackBeacon`, runtime, minimal dependency surface). `.gitignore` covers all
  Unreal build artifacts.
- **Logics layer (plain C++, no UE)** under `Public/BlackBeacon/Logics/`:
  `FBBBeamMath` (beam/cone query), `FBBRevealStateMachine` (reveal timing),
  `FBBObjectiveGraph` (prerequisite-aware chain resolver), `FBBWeatherInterpolator`
  (Clear/Fog/Rain/Storm). `TESTED` — 45/45 checks pass via `Tests/` (CMake +
  standalone harness, no engine, no external framework).
- **Interaction:** `IBBInteractableInterface`, `UBBInteractionComponent`
  (10 Hz forward trace, prompt focus), `UBBPromptWidget` (code-only Slate prompt).
- **Power:** `UBBPowerSystem` (world subsystem, event-driven source/consumer
  registry), `IBBPowerSourceInterface`, `IBBPowerConsumerInterface`,
  `UBBGeneratorComponent` (spin-up timer + interactable).
- **Lighthouse:** `ABBLighthouseController` (power consumer + beam control
  interactable), `UBBLighthouseBeamComponent` (rotation Off/Auto/Manual,
  intensity lerp, flicker, power state, publishes `FBBBeamQuery`,
  subscribes reveal components — the signature reusable system),
  `UBBBeamRevealComponent` (data-driven reveal over the state machine).
- **Weather:** `UBBWeatherController` (fog + weather states, exposes fog-density
  multiplier for beam/reveal readability).
- **Objectives:** `UBBObjectiveSystem` (game-instance subsystem over
  `FBBObjectiveGraph`, chain text-driven from `DefaultGame.ini`),
  `UBBObjectiveTriggerComponent` (generic volume/interact/event trigger).
- **Save foundation:** `UBBSaveGame` + `UBBSaveSubsystem` (0.1: world-state
  structs only; serialization planned for 0.2).
- **Core:** `ABlackBeaconGameMode` (defaults + bootstrap flag + fallback
  player start), `ABlackBeaconPlayerController` (code-created Enhanced Input,
  manual beam aim), `ABlackBeaconPlayerCharacter` (sprint/crouch + interaction),
  `ABlackBeaconGameState`, `UBBProceduralWorld` (greybox slice builder wiring
  the whole loop — runnable with zero authored assets).
- **Config:** `DefaultGame.ini` carries all tunables and the 9-step objective
  chain `BB_OBJ_ARRIVE` → `BB_OBJ_DISCOVER_ANOMALY`. Every ini section maps to
  a `config`-tagged `UPROPERTY` class (cross-check in `validate.sh`).
- **Tooling:** `Tools/validate.sh` — project-file sanity (`.uproject` JSON,
  ini section → class cross-check) + builds/runs the logic tests. `PASSES`
  green on this machine.

### Fixed during bootstrap (pre-commit review)

- Forward-declaration mismatches in `BBProceduralWorld.h` (weather/lighthouse).
- Removed a stale `BindInputs` declaration in `BBPlayerController.h`.
- Missing `CollisionQueryParams`/`Pawn`/`StringConv`/`Text`/`World` includes.
- Config key removed/renamed (`NominalWatts` dropped) so keys match
  `UPROPERTY(config)` names exactly.
- All `config`-tagged classes set to `config = Game` in their headers.
- Test-side bug in the reveal machine test (beam left unpowered) fixed; the
  machine logic itself was correct.

### Notes

- **No new dependencies.** No plugins, no third-party libraries, no services,
  telemetry, accounts, backends, or monetization.
- Beams/flicker/weather palette are `PLANNED` for depth beyond 0.1 defaults;
  audio and authored content are out of 0.1 scope.
- Full UE build/run status is tracked in `CURRENT_STATE.md`; M0.1 acceptance
  criteria live in `MILESTONES.md`.
