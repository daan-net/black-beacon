# V0.4 validation — 2026-09-25

**TESTED playable checkpoint.** Final rendered suite: **5/5**, zero test warnings
or errors, process exit 0. Original owner checkpoint `1c0cf71` and tag
`checkpoint-v04-playable-20260925` remain intact.

## Completed on resume

The recovery asset build had already succeeded; no full regeneration or shader
rebuild was repeated. Existing nine architectural meshes, material bindings,
1–7 draw sections per mesh, gallery collision and all eight approved reference
images were verified. The owner's improved geometry/materials were retained.

The first integrated run found one real regression: physical stair traversal
stalled at step 74/84. Pawn Z=1471.810 plus its 88 cm capsule half-height matched
the new gallery underside at Z=1560. Its XY position lay outside the 112-degree
hatch. The visible gallery and collision deck now share a 200-degree hatch,
clearing the approaching capsule. Only those two meshes were reimported; all
other audited asset/reference hashes match the protected checkpoint.

Added tests for that recorded capsule footprint/head position; relocated one
floor probe to the remaining supported gallery area. The final run physically
climbed all 84 steps and remained grounded at lantern height.

| Final check | Result |
|---|---|
| GameplayFlow: entry/objectives, generator, power, beam, reveal, save/load | PASS |
| PlayerControls: movement, sprint, crouch, look and stair pitch | PASS |
| StairTraversal: full physical ascent | PASS |
| StormWorld.Identity: native storm layers, route, flash/thunder, shelter | PASS |
| ArchitectureReview: single directional light, rotor, gallery, hatch, A–O | PASS |
| UE 5.8.2 editor target | BUILD SUCCEEDED |
| Standalone logic + geometry | 53 + 5 checks PASS |

This confirms the tested route, not every possible collision edge case or an
extended endurance session. No runtime blocker remains in that route.

## Evidence

- [Final automation report](Saved/Automation/VisualRebuildV04Clearance/index.json)
- [Final rendered log](Saved/VisualRebuildV04/Validation/GameplayClearance.log)
- [Original failure evidence](Saved/VisualRebuildV04/Validation/Gameplay.log)
- [A–O full-size screenshots](Saved/VisualRebuildV04/Validation/Screenshots.md)
- [Contact sheet](Saved/VisualRebuildV04/Validation/ContactSheet.jpg)
- [Before/after, matching camera](Saved/VisualRebuildV04/Validation/BeforeAfter.jpg)
- [Asset audit](Saved/VisualRebuildV04/Validation/AssetAudit.json) and
  [repaired gallery audit](Saved/VisualRebuildV04/Validation/GalleryAssetAudit.json)
- [Performance sample](Saved/VisualRebuildV04/Validation/Performance.json)

Screenshots are actual 1920×1080 Unreal renders. Montages only resize/arrange
frames; no exposure or scene-content edits. Weather timing differs between the
old/new comparison. A–O includes the full tower, exterior 3/4, base, annex inside
and outside, three stair views, lantern, Fresnel, beam off/on, sky, coast and reveal.

The short 1417-frame storm sample has median 20.23 ms, p95 25.64 ms and aggregate
44.93 FPS including screenshot stalls. Sustained 60 FPS is not established.
The older performance acceptance gate remains open.

## Visually weak areas

- Beam is still bright, dense and uniform in exterior/reveal views; the rebuilt
  shader alone has not met the soft atmospheric-scattering target.
- Close Fresnel shows an overbright spherical source and simplified optics;
  the mechanism needs authored detail and controlled exposure.
- Annex architecture improved substantially, but the generator still reads as
  primitive machinery and some wall surfaces become pale under warm practicals.
- Large rounded rocks, dark coastal masses and broad spray/foam patches need
  more convincing scale, placement and shoreline contact. The wreck is still
  a sparse hull/frame assembly.
- Stair structure is legible, but close surfaces remain repetitive and lighting
  varies sharply. Some fixed cameras are partly occluded (base/lantern views).

These are recorded art limitations, not reasons to resume micro-adjustments.
The structural improvement is visible; final production-art acceptance is not claimed.

## Recommended V0.5 — hero machinery and beacon optics

A coherent authored pass on the generator and lantern/Fresnel mechanism, with
beam scattering and warm practical lighting integrated around those assets.
Acceptance should focus on convincing close gameplay views and a beam that reads
as light in mist from both the tower and the reveal viewpoint. Keep the tested
architecture, traversal and gameplay contracts. Coast/wreck finish remains a
separate visual backlog; prioritize rather than opening many partial mechanics.

V0.5 is only a recommendation. No work or new scope has been started.

## Source maintenance

The reviewed OBJ/UE assets are authoritative. Some pre-pause generator edits were
never exported (see V04_RESUME.md and the source drift record). Reconcile that
source drift deliberately before a future full regeneration. It does not prevent
loading or playing this checkpoint and was not silently folded into this resume.
