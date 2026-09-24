# BLACK BEACON — Change Log

Format: `date — milestone — summary`. Version reflects the vertical slice
(`0.x`); no released build exists yet. A change that alters an existing
user-visible behaviour is called out as **user-visible**.

Status vocabulary used here (same as AGENTS.md): `IMPLEMENTED` (built in the
intended context), `TESTED` (built + exercised with evidence), `PLACEHOLDER`
(scaffolding to be replaced), `PLANNED` (paper only). Anything written-but-not-
compiled is marked **written, not built** and is *not* `IMPLEMENTED`.

## 2026-09-24 — M1 — runtime sky fill correction

- Made the weather controller's realtime-captured SkyLight movable so runtime
  intensity updates and storm-sky capture use a supported mobility mode.
- Added GameplayFlow assertions for the SkyLight mobility and nonzero runtime
  intensity. UE 5.8.2 Linux Development build succeeded; the full rendered
  Vulkan M01 suite passed 3/3 (GameplayFlow, PlayerControls, StairTraversal)
  with zero warnings/errors, and `./Tools/validate.sh` passed all 45 logic checks.
- Render review did not show a material coast-lighting improvement from this
  correction alone. The dark blockout coast and broad pale reveal wedge remain
  open M1 visual issues; no M1 completion is claimed.

## 2026-09-24 — M1 — focused beam and reveal review

- **User-visible:** Narrowed `BeamHalfAngleDeg` from 6 to 2.5 degrees. The
  configured angle drives the lighthouse spotlight, visible shaft, and
  `FBBBeamQuery`, so illumination detection remains synchronized with the
  rendered beam.
- Reframed the automated reveal capture to observe the wreck from the coast
  side rather than directly down the beam axis. This changes only test-camera
  placement. The rendered capture shows more of the wreck silhouette where the
  beam crosses it, though the shaft remains too pale and the wreck remains
  placeholder-quality.
- UE 5.8.2 Linux Development build succeeded; the full Vulkan M01 suite passed
  3/3 with zero warnings/errors, and `./Tools/validate.sh` passed all 45 logic
  checks. M1 remains open.

## 2026-09-24 — M1 — faceted coastal boulders

- Replaced the smooth sphere instances in the coastal rock field with the UE
  5.8.2 PCG sample `PCG_Boulder_02`, keeping the existing instancing, wet-basalt
  material, placements, and blocking collision. Scaled the sample mesh to 0.4
  of the previous blockout transform so the generator approach stays clear.
- Added a GameplayFlow assertion that the engine mesh loads. UE 5.8.2 Linux
  Development built successfully; the full rendered Vulkan M01 suite passed
  3/3 (GameplayFlow, PlayerControls, StairTraversal) with zero warnings/errors;
  and `./Tools/validate.sh` passed all 45 engine-independent checks. The opening
  capture now has faceted rock silhouettes; they remain low-poly placeholders,
  not finished coastal terrain. The mesh comes from UE's enabled-by-default PCG
  sample content; no PCG gameplay code or module dependency was added. The
  five-second automation warm-up measured 42.98 FPS and is not a gameplay
  benchmark.

## 2026-09-24 — M1 — frame the lighthouse on arrival

- **User-visible:** The real saved-map opening view pitched down two degrees,
  cropping the lantern room. Raised the configurable opening pitch to five
  degrees while retaining the lighthouse-facing yaw. GameplayFlow now checks
  that the starting view faces the lighthouse within one degree and tilts up
  enough to retain the lantern in frame.
- UE 5.8.2 Linux Development built successfully. The new rendered exterior
  capture includes the full lantern room, GameplayFlow passed, the full Vulkan
  M01 suite passed 3/3 with zero warnings/errors, and `./Tools/validate.sh`
  passed 45/45. The five-second automation warm-up measured 42.69 FPS; it is
  below the 60 FPS target and is not a gameplay benchmark. M1 remains open.

## 2026-09-23 — M0.2 — animate generator startup

- **User-visible:** The annex generator's flywheel now rotates in sync with its real running and spin-up state, accelerates as the four-second power ramp advances, and coasts down on shutdown. The environment binds to a new running-state event and updates the wheel on a 20 Hz timer only while it is turning. GameplayFlow asserts measurable wheel rotation during spin-up and coast-down. UE 5.8.2 built successfully; the full rendered Vulkan M01 suite passed 3/3 with zero test warnings/errors; `./Tools/validate.sh` passed with 45/45 logic checks. The five-second 1920x1080 automation warm-up measured 43.98 FPS, below the 60 FPS target and not a representative gameplay benchmark. The machine remains primitive greybox geometry.

## 2026-09-23 — M1 — lighthouse exterior readability

- **User-visible:** Added a generated weathered whitewash texture to the existing lighthouse tower skin and raised moon sky fill from 0.1 to 0.22 so the tower silhouette reads in the opening view. The material binding is asserted by GameplayFlow. UE 5.8.2 build succeeded; rendered Vulkan M01 automation passed 3/3 with zero test warnings/errors, and `./Tools/validate.sh` passed all 45 logic tests. The 1080p automation warm-up reached 44.77 FPS, below target and not a representative gameplay benchmark. The exterior remains blockout geometry; the close opening camera crops the lantern and the surrounding coast is still very dark.

## 2026-09-24 — M1 — first-reveal lighting review

- **User-visible:** Warmed the reveal tint for its rusted wreck surfaces. The rendered GameplayFlow still shows the revealed fragment as a dark silhouette beneath a broad, pale fog wedge. A temporary opacity/scattering adjustment had no material visual effect and was reverted. The final UE 5.8.2 build succeeded; the full Vulkan M01 report passed 3/3 with zero warnings/errors; `./Tools/validate.sh` passed 45/45. The automation process returned 1 during trace-server shutdown after all tests had completed successfully. Its five-second warm-up measured 46.15 FPS, below target and not a representative gameplay benchmark. The reveal remains mechanically `TESTED`; its visual quality remains `PLACEHOLDER` and does not pass the M1 gate.

---

## 2026-09-23 — M0.2 — shipwreck coast silhouette and atmosphere

- **User-visible:** Added 40 instanced Fresnel ridges across the eight lantern glass faces. The ridges share one non-colliding, no-shadow mesh component, use the warm `LensTint`/`LensIntensity` parameters, and switch off with beam power. The Vulkan exterior capture now reads as segmented lighthouse glass. UE 5.8.2 build succeeded; the full rendered suite passed 3/3 with zero warnings/errors; `./Tools/validate.sh` remains green at 45/45.

- **User-visible:** Corrected the lantern-pane and beam-lens material bindings. The authored `M_LanternLens` exposes `LensTint` and `LensIntensity`; the controller had been writing unrelated parameter names, leaving the panes clipped white. It now sets the real warm tint and a restrained runtime emission. Vulkan GameplayFlow passed and regenerated the exterior/beam captures; full PlayerControls and StairTraversal coverage remains from the immediately preceding 3/3 suite.

- **User-visible:** Added a continuous wet-basalt tower shell as a non-colliding child of the lighthouse controller. It covers the visual gaps between the serialized floor sections while preserving the interior stair route. The rendered opening capture confirms the tower silhouette; a gameplay test was updated to target the named lantern control after the new mesh changed component order.
- **User-visible:** Reduced configured moonlight and sky fill from 5.0/0.35 to 1.5/0.1 lux after Vulkan captures showed an overly bright blue night. This darkens the shoreline, though the generated storm panorama remains bluer and brighter than the supplied reference direction.
- UE 5.8.2 `BlackBeaconEditor` built successfully. The full Vulkan `BlackBeacon.M01` automation report passed 3/3 (GameplayFlow, PlayerControls, StairTraversal), zero warnings/errors, and regenerated the visual captures. The automation process returned 1 while UnrealTrace shut down after all tests had passed; the report itself records success. `./Tools/validate.sh` passes with 45/45 logic checks. The new shell and coast are still blockout geometry; no M1 work started.

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

## 2026-09-23 - M0.2 Storm Rain Presentation (Aesthetic Polish)
- Re-engineered weather rain presentation to use a dense 11x11 grid of lightweight fountain emitters tracking the camera to fill the local volume seamlessly.
- Applied Z-scaling to particles to simulate long, fast-moving rain streaks while maintaining RTX 2060 performance (using unscalable C++ attachments).
- Rotated particle emitters to point straight down, overcoming the fountain burst limits for sustained atmospheric downpours.
- Implemented per-emitter vertical raycasting to cull rain components explicitly over rooftops, stopping indoor rain natively without relying on a global toggle.

## 2026-09-23 — M0.2 — save/load restoration and rain review

- Completed exact runtime restoration for generator spin-up/production state, lighthouse startup and beam mode/aim, objective progress, persistent reveals, weather phase, and player transform. GameplayFlow now saves the shore checkpoint to disk, exercises the full objective/reveal path, reloads it, and checks restored state; the temporary automation slot is removed at test end.
- Added the objective readout and temporary save/load notifications to the HUD, and moved interaction prompts below the crosshair. Save/load are bound to F5/F9.
- Reduced rain sprite scale, emitter spacing, and camera-relative layer offset after user review found that the effect looked localized and was only apparent while looking upward. The regenerated Vulkan capture is still visibly clustered/chunky, so rain remains a `PLACEHOLDER` and its natural appearance is not accepted.
- UE 5.8.2 `BlackBeaconEditor` build succeeded. The rendered Vulkan M0.1 regression suite passed 3/3: GameplayFlow, PlayerControls, and StairTraversal. `./Tools/validate.sh` passed with 45/45 engine-independent tests. The five-second automation warm-up measured 43.37 FPS at 1920x1080, below the 60 FPS target and not a representative gameplay benchmark; M0.2 remains open.
- Repository hygiene: manual reference images/video and `.orig` backups were accidentally tracked in an earlier checkpoint. They remain present on disk and are now excluded from version control.

## 2026-09-23 — M0.2 — anchor rain in world space

- Removed the per-frame camera-relative repositioning that made precipitation move with the player. The Niagara emitters now keep deterministic, lightly jittered positions across the level around the weather actor, with configurable field radius and height. The roof probe still uses the player camera to suppress rain while indoors.
- GameplayFlow now records the rain field anchor and asserts that it remains stationary after player traversal. UE 5.8.2 `BlackBeaconEditor` built, `BlackBeacon.M01.GameplayFlow` passed with the new assertion and rendered storm captures, and `./Tools/validate.sh` passed 45/45.
- The field no longer follows the camera, but `FountainLightweight` still appears as distinct falling streaks in the rendered capture. Rain quality stays `PLACEHOLDER`; M0.2 remains open. Warm-up measured 42.33 FPS at 1920x1080 and still requires a representative performance pass.

## 2026-09-23 — M0.2 — replace point-source rain presentation

- Replaced the 11x11 Niagara fountain grid with one world-anchored instanced mesh field. Falling planes carry wind drift and deterministic variation, and their count tracks the existing weather intensity. Rain remains hidden when the existing upward roof probe detects overhead geometry.
- Added a project-owned soft streak texture and translucent material with tapered edges and ends. The beam/reveal weather state and objective logic were not changed.
- UE 5.8.2 `BlackBeaconEditor` build succeeded, `BlackBeacon.M01.GameplayFlow` passed 1/1 with regenerated exterior and indoor storm captures, and `./Tools/validate.sh` passed all engine-independent logic tests (45/45).
- The exterior captures show rain across the view without the previous point-source appearance or camera-following field. The intended indoor capture still shows rain, and the effect needs direct visual review. Keep rain `PLACEHOLDER` and M0.2 open. The automation's 1920x1080 warm-up measured 45.69 FPS for five seconds; this is below the target and not a representative gameplay benchmark.

## 2026-09-23 — M0.2 — allow downward look on stairs

- **User-visible:** replaced the stair camera pitch lock with a configurable -45 to +30 degree pitch range. Players can look down to check the steps while descending; horizontal steering and turning remain responsive.
- Updated `BlackBeacon.M01.PlayerControls` to check that pitch changes on a stair tread and remains within bounds. UE 5.8.2 `BlackBeaconEditor` built successfully. The Vulkan `BlackBeacon.M01.PlayerControls` test passed, including stair pitch response, pitch bounds, and yaw checks; `./Tools/validate.sh` passed all 45 logic tests. Direct player feel still needs review.

## 2026-09-23 — M0.2 — stair camera control

- On tagged stair treads, vertical mouse look initially held the camera at the configurable forward pitch while horizontal mouse look remained active. This was replaced by bounded vertical look in the follow-up entry above.
- `BlackBeacon.M01.PlayerControls` passed in UE 5.8.2 Vulkan with checks for stair-base detection, pitch lock, horizontal response, and unchanged ground look. The editor target builds and `./Tools/validate.sh` passes all 45 logic tests.
- Direct stair feel remains for user review. The user has accepted the revised rain appearance and confirmed that the lighthouse interior stays dry; the rain gate is now accepted. Coast rocks and wreckage still have no collision and remain a traversal follow-up. M0.2 remains open because the 60 FPS gate is not met by the latest non-representative 45.69 FPS warm-up.

## 2026-09-23 — M0.2 — generator annex readability

- Hid the old solid shed cube that obscured the traversable annex, widened its west doorway by 60 cm, and retained the blocking shell. The original generator mesh remains as an invisible blocking interaction collider; the real generator interaction and power flow are unchanged.
- Dressed the existing generator with a compact engine casing, cylinder head, skids, ribs, flywheel, hub, gauge, and regulators. Added a warm, unshadowed 850 lumen point light in the annex. These pieces use existing engine primitives and materials, so they remain a low-cost blockout rather than authored final art.
- Added in-engine assertions for the shell, hidden source cube, collider, machine details, and annex light, plus the rendered `BlackBeacon_M02_Generator.png` capture.
- UE 5.8.2 Linux Development build succeeded. The complete Vulkan `BlackBeacon.M01` suite passed 3/3 (GameplayFlow, PlayerControls, StairTraversal) with zero test warnings/errors; `./Tools/validate.sh` passed with 45/45 logic tests. M0.2 and visual art remain in progress.

## 2026-09-23 — M0.2 — distant reveal landmark

- Increased only the seven BeamReveal-owned ruin pieces to form a roughly 18 m tall silhouette that can be read from the lighthouse. The anomaly actor's location, hidden initial state, collision, beam query, and objective progression are unchanged.
- Improved the rendered test framing to capture the reveal from beside the lighthouse, with an offset view and narrower field of view so the distant silhouette is visible in the scene.
- UE 5.8.2 Linux Development build succeeded. The full rendered Vulkan `BlackBeacon.M01` suite passed 3/3 with zero test warnings/errors, including the new visible-size assertion; `./Tools/validate.sh` passed with 45/45 logic tests. The ruin remains greybox primitives, and the coast and lighthouse still need authored visual work.
## 2026-09-23 — M1 — beam-following first reveal (in progress)

- Inspected the existing gameplay flow before implementation: generator, lighthouse
  power, stair traversal, beam control, weather, save/load, and the first anomaly were
  already present. No duplicate systems were added.
- Extended the existing BeamReveal component to evaluate tagged static-mesh parts
  independently against the subscribed beam query. The current ruin's visible parts
  now follow the moving beam, fade after it leaves while power remains on, and hide
  after the fade. First discovery activates a shoreline search objective. Save restore
  resets this transient visual state.
- Corrected the existing BeamReveal config keys to the component's actual UPROPERTY
  names. The first reveal is transient by default; persistent behavior remains available
  through the existing property.
- Expanded rendered GameplayFlow assertions for partial reveal, beam departure fade,
  discovery progression, and transient reveal reset on load. The storm capture now waits
  for beam alignment and frames the reveal from near the lighthouse.
- UE 5.8.2 Linux Development build succeeded. The full Vulkan `BlackBeacon.M01` suite
  passed 3/3 after the final capture adjustment. `./Tools/validate.sh` passed with 45/45 logic tests.
- The generated `BlackBeacon_M01_D_Reveal.png` is an actual storm render. It confirms
  the fog beam but shows that the seven-piece ruin is still too dark and crude. This is
  a tested mechanic with placeholder art, not a completed M1 reveal or visual-review
  checkpoint.

## 2026-09-23 — M1 — wreck silhouette blockout

- Replaced the seven isolated ruin columns with a 15-part trawler wreck silhouette:
  hull, exposed ribs, deck beams, and a snapped mast. Each part remains tagged and
  revealed through the existing beam query; actor position, detection, and objective
  logic are unchanged.
- The storm D capture now reads more clearly as a wreck within the sweeping light.
  It remains primitive engine-cube geometry with unfinished materials and is still
  `PLACEHOLDER` art.
- UE 5.8.2 Linux Development build succeeded. Vulkan GameplayFlow passed with zero
  errors/warnings, including partial reveal, fade, objective progression, and restore.
  `./Tools/validate.sh` passed with 45/45 logic checks.

## 2026-09-23 — M1 — authored hull and reveal evidence

- Replaced the blockout-only hull with the 952-triangle Shipwreck Hull Section from
  3DAssets.dev, oriented broadside to the lighthouse beam. Retained separately tagged
  ribs and a broken mast so the existing BeamReveal query can reveal parts progressively.
- Added a project-generated weathered hull albedo and disabled unnecessary Nanite data
  on the imported meshes. This avoids a Nanite material fallback for the small mesh.
- Added matched real Vulkan captures from one camera with the reveal visible and after it
  fades: `BlackBeacon_M01_D_Reveal.png` and `BlackBeacon_M01_D_RevealOff.png`.
- Recorded the original GLB, CC0 source, generated texture provenance, and checksums in
  `ASSET_PROVENANCE.md`.
- UE 5.8.2 Linux Development build succeeded. The rendered Vulkan M01 suite passed 3/3
  with zero test warnings/errors; `./Tools/validate.sh` passed with 45/45 logic checks.
  M1 remains open because the surrounding environment and reveal audio are still
  placeholder/planned, and the five-second automation warm-up remains below 60 FPS.
