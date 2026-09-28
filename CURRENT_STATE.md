# BLACK BEACON — Current State

**Last updated:** 2026-09-28 · **Milestone:** Published Linux playtest distribution

## Asset-first environment kit v1 — written, not built

Owner authorized an asset-first cinematic environment pass on
`work/asset-first-environment-pass`, preserving main and the released package.
Audit, category acquisition plan and provenance are in `Docs/`. Four official
Poly Haven CC0 assets acquired with verified checksums; new materials and visual
integration are being built. No legacy assets, gameplay collision or global
render settings are changed. Build/render validation is pending; no new Linux
package or installer verification is part of this art pass.

## GitHub installation infrastructure — IMPLEMENTED / TESTED publicly

Root `install.sh` and `uninstall-black-beacon.sh` support checksum-first staged
Linux x86_64 installation, atomic application switching, save/config preservation,
legacy migration and desktop/command launchers. All 13 distribution tests pass,
including extraction of the unchanged real r2 archive. ShellCheck, desktop-entry
validation and `Tools/validate.sh` pass. No game rebuild/repackaging or art/gameplay
changes; no CI/Actions. Full report: [DISTRIBUTION.md](DISTRIBUTION.md).

Published public `daan-net/black-beacon`, default branch `main`, prerelease/tag
`linux-test1-r2` at `121cd4c1a857f718e2a08414ae988a230e7ac46a`. All nine branches
and ten existing tags were pushed intact; no history was rewritten. Both existing
release assets have matching GitHub/local sizes and SHA-256 digests.
The exact public curl installer passed in an isolated temporary HOME: anonymous
download, checksum before extraction, command/desktop launchers, actual packaged
headless startup (exit 0), and repeat installation retaining save data and one
application version. This did not repeat rendered gameplay validation.
GitHub Actions is explicitly disabled. No rebuild/repackaging or gameplay/art edits.
Pre-existing untracked `Tools/cook_r2.sh` and `run_10m.sh` remain untouched.

## Linux Tester Package - IMPLEMENTED / TESTED

Created standalone Linux x86_64 build and installers (`Tools/install-test-build.sh`, `Tools/uninstall-test-build.sh`). Successfully validated deployment, updates, uninstallation, and checksum verification in an isolated mock environment. The packaged game starts cleanly without UE dependency, retaining save games on updates. Output artifacts placed in `dist/` (not committed). Added `TESTER_LINUX.md` and `LINUX_PACKAGE_VALIDATION.md`.

## Hero Lighthouse Art Pass V0.1 — IMPLEMENTED / TESTED technical art candidate

Owner authorized continuation after recovery. V0.5.1 is preserved at `06a986f`,
tag `checkpoint-v051-playtest-candidate-20260926`, with both final gameplay tests
passing. The new pass adds separate exterior architectural modules and an isolated
exterior finish; it does not regenerate historical V0.1/V0.4/V0.5/access assets.
No new gameplay or coastline milestone is included. Three imported meshes add
34,968 triangles of ashlar base/portal, curved cast gallery corbels and window
dressings. An isolated material replaces only the exterior paint slot. Blender
5.0.1 source is saved alongside Python/OBJ/HLSL. UE build/import succeeded;
`Tools/validate.sh` passes with seven geometry checks. Final Vulkan review passes (five native 1080p captures); V0.5.1 PlaytestLoop
and M01 GameplayFlow pass in NullRHI, all exit 0. M01 now checks the exact new
exterior finish and preservation of every other imported slot rather than the
obsolete `M_LH_` name prefix. The final surface-only correction was reviewed in
Vulkan; no traversal/reveal implementation changed. Owner art/UX acceptance remains
pending. See [art report and captures](HERO_LIGHTHOUSE_ART_V01.md).

## V0.5.1 — IMPLEMENTED / TESTED technical candidate

On `work/v051-playtest-candidate`, preserving safety baseline `a036ba4` and V0.5
checkpoint `d1bbab5`. The entrance obstruction was the first `BB_StairGuard`'s
invisible projecting corner. Only its entrance-facing half was trimmed to end
at the existing first post. Door geometry, tower shell, upper guards and
traversal dimensions remain intact. No failed door-leaf experiment remains.

Unreal 5.8.2 build and `Tools/validate.sh` pass. Final targeted
`BlackBeacon.V051.PlaytestLoop` and `BlackBeacon.M01.GameplayFlow` both pass,
exit 0. Entry at -3/0/+3 degrees passes the original 0.6-second deadline;
full climb, balcony access, generator, yaw/pitch aiming, manual wreck discovery,
release/reacquire, objectives and save/restore are exercised.

[Root cause, validation evidence and controls](V051_VALIDATION.md).
Owner manual UX acceptance remains pending. Existing V0.4/V0.5 hero assets and
V0.5.1 access source/imports are intact; no partial art export was found during
recovery. The reference package's V0.1 implementation record is historical and
must not overwrite these later assets.

## V0.5 — TESTED hero environment playable checkpoint

Completed on `work/v05-hero-environment` from validated `cf3923a`. V0.4 tags
remain unchanged. No experimental branch content was merged.

- **IMPLEMENTED / TESTED:** authored lantern bays/dome, prism optics, compact arc,
  level rotating carriage, supported drive and instrument console; engine,
  alternator, flywheel, switchboard and service lamp; new surface/weathering and
  softer finite beam scattering. Six source/Unreal mesh modules, eight new Unreal
  mesh/material assets. Editable Python/OBJ/MTL/HLSL sources, no Blender file.
- **TESTED:** final UE 5.8.2 / Vulkan / native 1080p suite **5/5**, zero test
  warnings/errors, exit 0. Launch, generator interaction, all 84 stair rises,
  gallery/hatch, lantern access, beacon control/rotation, reveal/fade, objectives
  and save restoration pass. Exactly one directional light remains.
- UE editor build succeeded; `./Tools/validate.sh` passes 53 logic and five geometry
  checks. Gallery/deck and all eight reference PNGs match `cf3923a` byte-for-byte.
- Review corrections replaced obsolete prototype assertions, supplied explicit
  normals, corrected emitter/glass shadows and moved opaque machinery/stairs to
  Nanite. The observed VSM overflow did not recur in the final complete run.
- Final report: `Saved/Automation/VisualRebuildV05Release/index.json`.
  Fifteen actual gameplay captures, six-view sheet and matching-camera comparison:
  `Saved/VisualRebuildV05/Validation/`. Full findings: `V05_VALIDATION.md`.

No known blocker remains in the tested route. Visual limits remain: regular lens
courses/approximate optics, simplified machinery and dark lower faces, procedural
wall weathering, weak coast/foam/wreck assets. No production-art or sustained
60-FPS acceptance is claimed. Recommended next: V0.6 coastal approach and
shipwreck hero pass; not started. Preserve this stable V0.5 state.

## V0.4 — TESTED playable checkpoint; validation complete

Resumed from the owner's manually reviewed `1c0cf71`, preserved at
`checkpoint-v04-playable-20260925`. The old pause instructions were superseded by
its successful recovery Vulkan asset build. No blanket regeneration or material
rebuild was performed during validation.

The first rendered run found an actual upper-stair obstruction at step 74/84.
Only the matching gallery visual and collision hatch were widened 112→200 degrees;
original stairs, gameplay logic and all materials remain unchanged. Added a
capsule-footprint regression and an in-engine probe at the recorded obstruction.

- **TESTED:** final 1920×1080 Vulkan suite **5/5**, zero test warnings/errors,
  exit 0: GameplayFlow, PlayerControls, StairTraversal, StormWorld.Identity,
  ArchitectureReview. Entire 84-step climb reaches lantern height grounded.
  Generator/power, interactions/objectives, beam start/aim/rotation, reveal/fade,
  and save/load restoration pass. Exactly one directional light is asserted.
- **TESTED:** `./Tools/validate.sh` passes the existing 53 logic checks and five
  geometric regression checks. UE 5.8.2 BlackBeaconEditor build succeeded.
- Read-only audit: all nine meshes load, material slots resolve, 1–7 sections
  per mesh, correct gallery collision mode, all eight approved reference PNGs.
  Hash comparison confirms only the two intended gallery OBJ/UE meshes changed.
- Fifteen real A–O captures, contact sheet and same-camera before/after comparison:
  `Saved/VisualRebuildV04/Validation/Screenshots.md`.
- Final automation report: `Saved/Automation/VisualRebuildV04Clearance/index.json`;
  logs/audits: `Saved/VisualRebuildV04/Validation/`. The first failed run is retained
  separately as `Gameplay.log`; the passing run is `GameplayClearance.log`.
- Short storm sample, 1417 frames: median **20.23 ms**, p95 **25.64 ms**,
  aggregate **44.93 FPS** including screenshot stalls. This is not sustained
  60 FPS or a long-duration stability certification. M0.2 performance gate stays open.

No known blocker remains in the tested gameplay route. Visual weaknesses remain:
beam still reads dense/uniform, close Fresnel is overbright, generator machinery
is primitive, coastal rocks/foam lack convincing contact and scale, and some
warm wall pools show repetitive surface detail. V0.4 is a validated playable
checkpoint, not final production-art acceptance. No micro-polish or V0.5 work
was started during that V0.4 validation session. See `V04_VALIDATION.md` for findings and the recommended next milestone.

Source reproducibility caveat: committed OBJ/UE assets define this checkpoint;
pre-pause generator-only variations remain unexported. `V04_RESUME.md` records
them so a later full regeneration does not silently replace reviewed art.

## Storm World Identity V0.1 — TESTED checkpoint; USER VISUAL / AUDIO REVIEW

Authorized by the milestone owner on 2026-09-24. This pass is built and exercised
in the actual saved map on UE 5.8.2 / RTX 2060 / Vulkan SM6. It adds:

- Native SkyAtmosphere / moving VolumetricClouds / cloud shadows / sky fill and
  height fog. The retained static panorama is hidden during play. Moonlight is
  2.5 lux, sky fill 1.1; exposure and the existing beam/reveal fog contract stay intact.
- A displaced, opaque sea with three irregular swells and crest foam; finite
  coastal terrain retains the shore-to-lighthouse route at Z=0. Seven impact
  sites supply staggered spray and drifting local mist; 96 ground splash cards.
- Shared wind at 22 degrees, gust-modulated 1400 cm/s maximum base drift, used by
  rain travel/orientation, spray, clouds and sea. The existing shelter test keeps
  rain out of the lighthouse. Storm is now the initial weather preset.
- Infrequent lightning, distance-delayed thunder, wind/rain/surf/thunder audio,
  positional surf and thunder, and shelter-driven volume / low-pass changes.
  Audio sources are original synthesized sounds; subjective quality needs listening.
- Locally damp nonmetallic ground and the existing localized hero wetness materials.
  Existing generator, traversal, power, controls, objectives, save/load and beam
  detection remain authoritative; no new gameplay mechanic was introduced.

### Validation and renderer compatibility

- `Saved/StormReview/Build.txt`: BlackBeaconEditor Linux Development **Succeeded**.
- `./Tools/validate.sh`: **VALIDATION OK**; standalone tests **53/53** (the existing
  45 plus eight storm timing / gust checks).
- Two consecutive native-1920×1080 rendered `BlackBeacon` suites passed **4/4**,
  zero automation warnings/errors, exit 0. Logs/reports are
  `Saved/StormReview/Passed_DescriptorHeap_Run{1,2}.{log,json}`. They exercise
  GameplayFlow, PlayerControls, StairTraversal and StormWorld.Identity.
- The storm test checks native assets, diagonal wind, original panorama hidden,
  three route floor probes, real generator/beam activation, flash/thunder delay,
  and interior audio shelter response. Existing GameplayFlow checks reveal/fade,
  objective progression and save restore; StairTraversal walks the 84 steps.
- During integration the default Vulkan `VK_EXT_descriptor_buffer` backend
  intermittently lost the GPU device in cloud/fog/shadow passes. Alternative
  cloud-shadow and async-compute experiments did not resolve it and were reverted.
  `r.Vulkan.Bindless.PreferredExtension=1` selects the installed engine/driver's
  supported `VK_EXT_descriptor_heap` backend; startup logs confirm that selection.
  Both final runs passed with native clouds/shadows and normal async compute.
  This is a tested compatibility workaround on NVIDIA 595.91.07, **not proof of
  the underlying driver/engine cause or long-duration stability on other hardware**.
  Routine engine startup warnings still exist; zero warnings refers to test results.

### Real evidence and performance

- Eleven actual Unreal screenshots: `Saved/Screenshots/LinuxEditor/BlackBeacon_Storm_*`:
  A lighthouse/sky, B sea, C wet rocks, D rain, E coastal mist, F lightning,
  G beacon/fog, H interior, I night, J wreck reveal, A2 repeated sky view.
  `Saved/StormReview/ContactSheet.png` is only a montage of those rendered frames.
- Actual runtime mix: `Saved/StormReview/BlackBeacon_Storm_RuntimeMix.wav`:
  32.85 seconds, 48 kHz, six channels, peak 0.594, no PCM clipping. This verifies
  audio output exists; it is not a human judgement of mix quality.
- Latest CSV sample: `Saved/Profiling/CSV/Profile(20260924_190843).csv`, 1615 frames,
  median 16.81 ms, p95 22.96 ms, aggregate 50.26 FPS including screenshot stalls.
  Sampled total GPU memory peak 3363 MiB. Metrics: `Saved/StormReview/Metrics.json`.
  This short automated sequence does **not** establish stable 60 FPS gameplay.

### Remaining review gates

Inspected renders show layered storm clouds, rough water/foam, angled rain, coastal
spray, a readable lighthouse and a visible lightning event. The coast still has
faceted blockout boulders; close interior materials remain pale/stretched with a
checker-pattern ceiling. The beam remains too dense when viewed along its axis
(J); G shows the beam-to-wreck relationship more clearly. These are recorded
limitations, not final art acceptance. Test cloud/wave motion, rain, audio balance,
and the full player route interactively before accepting the storm's visual identity.
**M0.2 and M1 remain open. Stop here for USER VISUAL / AUDIO REVIEW.**

Sources and regeneration: `Art/Source/StormWorld/README.md`. Review command:
`./Tools/review_storm.sh`. Pre-change recovery archive:
`Saved/Checkpoints/Before_StormWorld_V01.tar.gz`, source checkpoint `ed707a5`.
The two pre-existing modified texture assets and eight supplied reference PNGs
were preserved and excluded from this pass's commit.

## Earlier verified implementation (historical evidence)

| Item | Status | Evidence |
|---|---|---|
| UE 5.8.2 Linux editor target | `TESTED` | `BlackBeaconEditor Linux Development` UHT, Clang compile, and link succeeded after Phase B changes. |
| Actual project editor startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -log -NoSplash` initialized, loaded Entry, and passed map check (0 errors, 0 warnings); it remained open for the planned 70-second smoke run. |
| Rendered game startup | `TESTED` | `UnrealEditor BlackBeacon.uproject -game -log` initialized on the NVIDIA RTX 2060 Vulkan device and remained up for a 45-second smoke run without a fatal error. |
| Player walk, sprint, crouch, look inputs | `TESTED` | In-engine `BlackBeacon.M01.PlayerControls` succeeded with injected W, Shift, C, MouseX, and MouseY events; it checked movement, sprint/crouch state, capsule height, eye height, and bounded camera yaw/pitch response. |
| Opening shore framing | `TESTED` | The real saved-map spawn is at (-4500, -600) cm; the initial view now pitches up 5 degrees while staying aligned to the lighthouse. GameplayFlow asserts the view is within 1 degree of the lighthouse azimuth and at least 4 degrees upward. The 1920×1080 Vulkan opening capture now includes the lantern room (`Saved/Screenshots/LinuxEditor/BlackBeacon_M01_A_Exterior.png`). The environment itself remains placeholder quality. |
| Generator interaction usability | `TESTED` | The generator greybox was raised from a partly buried 110 cm top to a 180 cm top, and the camera interaction query now uses a configurable 24 cm sphere sweep. GameplayFlow focuses and starts it while aiming naturally at camera height; the first blocking surface still prevents interaction through walls. |
| Generator annex entry and machine presentation | `TESTED` | Hid the solid shed source cube that had filled the traversable annex, widened its west doorway by 60 cm, and retained the blocking wall shell and original generator interaction collider. The real generator now has a dark engine casing, cylinder head, skid/ribs, flywheel, hub, gauge, regulators, and a warm 850 lumen unshadowed annex light. GameplayFlow asserts the shed is hidden, the machine details do not block movement, the interaction collider remains active, the light exists, and the real generator still starts. The rendered Vulkan capture is `Saved/Screenshots/LinuxEditor/BlackBeacon_M02_Generator.png`; this is still primitive-based blockout art, not final asset quality. |
| Generator motor feedback | `TESTED` | The generator flywheel now starts moving with the actual running/spin-up state, accelerates with its real 4-second progress, and coasts down when stopped. This uses one 20 Hz timer only while the motor is active or coasting. The rendered GameplayFlow test verifies rotation during spin-up and coast-down; the full Vulkan M01 suite passed 3/3 with zero test warnings/errors. |
| Distant anomaly landmark | `TESTED` | The BeamReveal-owned anomaly combines a 952-triangle CC0 ship hull with ten individually revealed frame/mast pieces. The hull is oriented broadside to the lighthouse beam and uses a project-generated weathered hull albedo. Its actor position and beam detection are unchanged. UE 5.8.2 Vulkan GameplayFlow verifies tagged-part reveal and fade; paired 1920×1080 renders show the wreck under the beam and absent after it leaves (`BlackBeacon_M01_D_Reveal.png`, `BlackBeacon_M01_D_RevealOff.png`). It remains a low-poly/blockout hybrid, not final art. Source and licence are recorded in `ASSET_PROVENANCE.md`. |
| Beam-following first reveal | `TESTED` | The existing BeamReveal system evaluates tagged parts against the existing beam query, reveals intersected pieces, fades them when the powered beam leaves, and records `BB_OBJ_APPROACH_REVEAL`. Save restore resets the transient visual. GameplayFlow verifies the live shaft material opacity/cone parameters, reveal and fade, objective progression, and save restore. The rendered diagnostic set compares the normal 0.04 shaft opacity against 0.0000001 and zero, plus spotlight scattering off (`BlackBeacon_M01_D_Reveal.png`, `..._ShaftOpacity0000001.png`, `..._NoShaftMesh.png`, `..._NoSpotScatter.png`). These controls change the frame only slightly; the broad, uniform pale shaft remains. A 3-degree total beam cone was rejected because the real flow no longer discovered the target; the synchronized 5-degree total cone is restored. The beam's rendered shape now needs a human visual review before another shader/material pass. Storm uses dark blue-gray fog rather than its former default light-gray tint. M1 remains open. |
| First-person and stair look controls | `TESTED` | Mouse pitch remains active on open ground. On tagged stair treads, vertical look is clamped to a configurable -45 to +30 degree range so players can inspect steps while descending; yaw remains responsive for steering and turning around. UE 5.8.2 Vulkan `BlackBeacon.M01.PlayerControls` passed with stair pitch response/bounds and yaw checks. Direct player feel still needs review. |
| Greybox stair lighting | `TESTED` | Three warm unshadowed point lights, one per tower floor, are built and exercised by the rendered M0.1 suite. They improve route visibility with no new collision or gameplay dependency; final brightness remains a human playtest judgement. |
| Objective, interaction, power, beam, reveal loop | `TESTED` | In-engine `BlackBeacon.M01.GameplayFlow` succeeded in `-game` with NullRHI and with Vulkan on the RTX 2060. It checks actual pawn overlaps, sweep-based generator and lantern interactions, prompt changes, four-second spin-up and power loss, manual beam aim, BeamReveal discovery, and objective progression. Reveal persistence is configurable; the first M1 reveal is now transient by default. |
| Physical greybox stair climb | `TESTED` | Review of `Debug_Manual/Screencast From 2026-09-23 08-58-12.mp4` exposed an awkward doorway gap and treads intersecting the central column. The route retains 84 steps, 28 per floor, and 520 cm rise per floor; current floor radii/depths are 190/130, 155/110, and 130/90 cm to fit the tapered tower. `BlackBeacon.M01.StairTraversal` reached lantern height and remained grounded. |
| Configured rain fog and spotlight settings | `TESTED` | GameplayFlow checks fog, spotlight scattering, visibility, and gameplay beam direction. The analytic shaft uses wider silhouette falloff, low-frequency world-space mist variation, and a warmer linear light colour based on the local `Manual_Visuals` references. Spotlight intensity is 1200 lumens. Five rendered captures show a clear B_Air versus B_AirOff difference, and direct review of the supplied moving video confirmed that the beam starts, reads, and rotates correctly. |
| Runtime sky fill setup | `TESTED` | The weather controller's realtime-captured SkyLight is now explicitly movable, matching its runtime intensity and moving storm dome. GameplayFlow asserts mobility and nonzero intensity. UE 5.8.2 compiled; the full rendered Vulkan M01 suite passed 3/3 (GameplayFlow, PlayerControls, StairTraversal) with zero warnings/errors, and `./Tools/validate.sh` passed all 45 logic checks. A rendered comparison did not show a material improvement to the coast from sky fill alone; the current exterior remains too dark and the reveal still has a broad pale fog wedge. |
| M0.1 in-engine regression | `TESTED` | After the video-driven stair correction, UE 5.8.2 `BlackBeaconEditor` built and the full rendered Vulkan `BlackBeacon.M01` suite passed 3/3 on the RTX 2060: GameplayFlow, PlayerControls, and StairTraversal, with zero warnings or errors. `./Tools/validate.sh` also passed all 45 logic checks. |
| Persistent M0.2 map | `TESTED` | `/Game/BlackBeacon/Maps/L_BlackBeacon_M02` is now the game and editor startup map. It contains the verified slice actors and PlayerStart, while runtime procedural bootstrap is disabled. The complete rendered Vulkan M0.1 suite passed 3/3 from this saved map with zero warnings or errors. |
| Shipwreck opening and lighthouse silhouette | `TESTED` | `ABBCoastalEnvironment` now instances the UE 5.8.2 PCG sample boulder mesh with the project's wet-basalt material in place of smooth engine spheres. A 0.4 mesh scale keeps the existing coastal route and generator interaction clear; GameplayFlow checks that the intended mesh loads and the full M01 suite passes. Boulders retain blocking collision. The shore is still low-poly placeholder geometry, and the dark exterior still needs authored terrain and lighting work. |
| Storm rain presentation | `TESTED` | Replaced 121 point-source Niagara emitters with a deterministic, world-anchored instanced field of 10,000 falling planes. A project material and alpha texture taper each streak at its edges and ends; fall direction, wind drift, weather intensity, and the roof probe remain connected to weather state. GameplayFlow passed and two exterior Vulkan captures show precipitation across the view without camera-following emitters. Direct user play confirmed that rain reads correctly and the lighthouse interior stays dry. |
| Save/load restoration | `TESTED` | `BlackBeacon.M01.GameplayFlow` writes a real save slot at the shore, completes the generator/beam/objective/reveal flow, then reloads and verifies the stopped generator, unpowered lighthouse, beam off, restored current objective, hidden unrevealed anomaly, and shore player position. The automation slot is deleted at test end. |
| Performance sanity | `PLACEHOLDER` | The latest 1920×1080 Vulkan automation warm-up measured 43.24 FPS for five seconds during the full regression run. This is not a representative gameplay benchmark and does not meet the 60 FPS target; profile and retest before closing M0.2. |
| Plain C++ logic and project sanity | `TESTED` | Current storm checkpoint: project checks green; 53/53 logic tests pass. |
| Ref-inspired environmental visual pass | `PLACEHOLDER` | Wet basalt texture, storm panorama, octagonal lantern glazing, a continuous tower silhouette, dressed generator machine, faceted engine-sample coastal boulders, and a low-poly imported hull are built and exercised. Tower, generator annex, lantern machinery, wreck frame/mast, and stairwell still rely partly on engine basic shapes; the shoreline is not final art. The reference imagery is direction, not evidence of final art quality. |
| Lighthouse exterior paint pass | `TESTED` | Added a project-generated weathered whitewash albedo to the existing tower skin and raised the sky fill from 0.1 to 0.22 so the tower remains readable against the storm night. The rendered first-person shore capture `Saved/Screenshots/LinuxEditor/BlackBeacon_M01_A_Exterior.png` confirms the texture is loaded and visible; GameplayFlow asserts its material binding. UE 5.8.2 build succeeded, full Vulkan M01 suite passed 3/3 with zero test warnings/errors, and `./Tools/validate.sh` passed 45/45. The render still shows a very dark, primitive coast and the lantern is cropped at this close view; this is a readability pass, not finished environment art. |
| Hero lighthouse V0.1 integration | `TESTED` | Five imported visual-only modules (tapered tower shell, gallery, lantern room/Fresnel cage, annex facade details, and basalt contact ring) are attached to the existing lighthouse/coastal actors. Generator interaction and objective trigger are moved beside the tower; existing functional actors remain authoritative. Rendered Vulkan automation passed GameplayFlow, PlayerControls, and StairTraversal 3/3 with zero test warnings/errors; new frames are in `Saved/Screenshots/LinuxEditor/BlackBeacon_Hero_*.png`. This verifies mesh loading and gameplay integration, not production art quality: stair framing reads mostly as an exterior sightline and close powered lens exposure is blown out. Imported material families are fallbacks; the contact rocks and interior remain rough. User visual review is pending. |
| Hero lighthouse material / UV correction | `TESTED` | Removed the Interchange-generated Phong parents (high Ks/Ns) from all six imported lighthouse material instances and reparented them to opaque project PBR masters. Tower paint is metallic 0 / roughness 0.84; aged iron is 0.12 / 0.72; brass is 0.82 / 0.43; wood is 0 / 0.76; rock is 0 / 0.62. Only the separate Fresnel lens effect is additive/unlit; tower windows and imported material families remain opaque. Albedo textures use sRGB, default color compression, and world mips. Audited assets contain no ORM/normal maps, so no packed channels were reversed or misconnected; surface relief currently comes from mesh normals and albedo only. Generated cylindrical tower UVs now tile at about 2 m, and box faces use dimensional face UVs. A world-normal / object-height mask localizes wet roughness and darkening without changing Metallic. Lowered practical stair-fill from 350 to 180 lm and its radius from 680 to 460 cm; increased night sky fill 0.22 to 0.28 and reduced lens/glazing highlights. Rendered captures are `BlackBeacon_Material_01_StairwellUp.png` through `..._05_LanternRoom.png`. The full Vulkan suite passed 3/3 with zero warnings/errors and `./Tools/validate.sh` passed 45/45. Exterior paint and rock now read rougher and their texture scale is improved, but close interior walls remain pale under practical lights and lantern-room capture is mostly dark; visual sign-off is still pending. |
| Audio implementation | `TESTED` | Four synthesized storm layers, spatial surf/thunder and shelter muffling; actual mix recorded. See current storm evidence above. Bespoke machinery/reveal audio remains future work. |

## Earlier M0.1 acceptance and material checkpoint

M0.1 remains accepted. M0.2 is **in progress** and remains open for performance, human traversal review, and substantial authored visual work. The user has authorized work toward the first M1 vertical-slice loop, including one reveal target and the small surrounding environment, while existing systems are audited to avoid duplicates. The UE 5.8.2 Linux Development build succeeded; `./Tools/validate.sh` passed with 45/45 logic checks; and the full rendered Vulkan M01 suite passed 3/3 with zero test warnings/errors. GameplayFlow verifies transient part reveal, fade on beam departure while powered, next-objective progression, and save/load reset. It also confirms runtime shaft opacity and cone width arrive at the assigned dynamic material. Matched storm renders show the authored low-poly hull and reveal-off state from one camera. Storm fog now keeps a dark blue-gray tint at its original dense setting, reducing the washed-out gray background. This is tested first-reveal functionality with placeholder/blockout surroundings; it is not M1 completion. The latest five-second Vulkan automation warm-up measured 43.24 FPS, below the 60 FPS target and not a representative gameplay benchmark.

The UE 5.8.2 installed build is `/home/a1/WORK/_TOOLS/UE_5.8.2` (`~/UnrealEngine` points to it). The latest full Vulkan M01 run (`Saved/Automation/M1_MaterialCorrection_ReleaseCandidate/index.json`) passed 3/3 with zero test warnings/errors and regenerated all five material views plus the hero, generator, beam, storm, and reveal captures. `./Tools/validate.sh` passed all 45 logic checks. The material audit found the imported Phong response and the stretched cylindrical UVs; both are corrected. The interior remains too pale in close views and the lantern-room material view is dark, so this is a verified correction pass rather than visual sign-off. The hero lighthouse remains V0.1; M1 and M0.2 remain open. Engine, build, logs, screenshots, and temporary test output are excluded from git.
