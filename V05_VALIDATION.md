# BLACK BEACON V0.5 — hero environment checkpoint

**TESTED**, 2026-09-25. Branch: `work/v05-hero-environment`, baseline `cf3923a`.
The V0.4 validated/playable tags are unchanged. No experimental branch was merged.

## Visible result

- Sixteen framed lantern bays, copper dome with ceiling ribs, open roller support,
  cast pedestal, geared drive and instruments replace the previous sparse housing.
- Compound prism drum and forward concentric lens surround a small power-driven
  arc source. Brass, dark iron, enamel and glass are separate surfaces. The carriage
  follows beam azimuth while staying seated on the horizontal bearing.
- A softer, uneven shaft fades along its length. The physical spotlight remains;
  its previous excessive fog scattering is reduced. Query range/angle and reveal
  thresholds are unchanged.
- The generator reads as an engine coupled to an alternator: finned cylinders,
  cast heads/crankcase, filters, fuel/exhaust lines, spoked flywheel, instrument
  switchboard and suspended service lamp. The existing annex shell is retained.
- Interior masonry courses, modeled stair lamps, restrained runoff/oxidation,
  cast enamel and tread surfaces replace the repetitive wall ripples and uniform
  metal finish. The approved eight-image reference package remains unchanged.

## Sources and Unreal content

Editable Python/OBJ/MTL/HLSL source:
[HeroEnvironmentV05](Art/Source/HeroEnvironmentV05/README.md). Blender is absent;
no `.blend` asset is claimed. Six generated meshes have explicit normals and
separate material sections. V0.4 gallery/deck meshes were not regenerated.

Changed meshes: `SM_BB_LH_LanternRoom`, `SM_BB_LH_FresnelRotor`,
`SM_BB_LH_StairStructure`. New meshes: `SM_BB_LH_ArcSource`,
`SM_BB_GeneratorWorks`, `SM_BB_GeneratorFlywheel`.

New materials: `M_LH_EngineEnamel`, `M_LH_RoofCopper`, `M_LH_DialFace`,
`M_LH_StairIron`, `M_V05_OpticalGlass`. Changed: shared weathered parent,
tower paint/interior plaster/iron/brass instances, and `M_BeamShaft`.
Assets reside in `/Game/BlackBeacon/Art/Lighthouse/` and the existing beam path.

All added visual components are nonblocking. The original generator collider,
stair collision, gallery hatch, controls, objectives and gameplay beam query stay
authoritative. The existing generator timer still drives its flywheel.

## Validation evidence

- UE 5.8.2 Linux Development editor build succeeded.
- `./Tools/validate.sh`: **53 logic checks and five geometry checks pass**;
  stair-height checks now exercise the V0.5 source.
- Final native 1920×1080 Vulkan run on RTX 2060: **5/5 pass**, zero test warnings
  or errors, process exit 0, 152.12 seconds. Tests: GameplayFlow, PlayerControls,
  StairTraversal, StormWorld.Identity, V05.ArchitectureReview.
- Exercised launch, actual generator interaction and power sequence, all 84 stair
  rises, upper hatch/gallery access, beacon interaction/control, rotation and aim,
  shipwreck reveal/fade, objectives and save restoration. One directional light.
- [Final automation report](Saved/Automation/VisualRebuildV05Release/index.json)
  and [runtime log](Saved/VisualRebuildV05/Validation/GameplayRelease.log).
- [Protected asset hashes](Saved/VisualRebuildV05/Validation/ProtectedAssets.json)
  prove both gallery meshes and all eight reference PNGs match `cf3923a` exactly.
- Mesh/material import audit and shadow/Nanite settings are under
  `Saved/VisualRebuildV05/Validation/`. Opaque machinery/stairs use Nanite;
  bulb and glass sections do not cast opaque shadows.

The first run exposed four obsolete assertions about the removed eight-pane/
40-band prototype. Replacement checks verify the real housing/glazing and arc
in off, on and power-loss states. Essential gameplay assertions were retained.
The following review exposed a VSM marking overflow; the final Nanite/shadow
correction eliminated that warning in the complete final run. Earlier logs and
captures are retained separately. No sustained-60-FPS or endurance claim is made.

## Actual gameplay screenshots

These are review cameras in the running game, with ordinary scene lighting.
No offline render, exposure retouching or image generation was used.

1. [Lighthouse exterior](Saved/VisualRebuildV05/Validation/Captures/BlackBeacon_V05_B_HeroThreeQuarter.png)
2. [Stair/interior](Saved/VisualRebuildV05/Validation/Captures/BlackBeacon_V05_F_LowerStairs.png)
3. [Generator annex](Saved/VisualRebuildV05/Validation/Captures/BlackBeacon_V05_E_AnnexInterior.png)
4. [Lantern room wide](Saved/VisualRebuildV05/Validation/Captures/BlackBeacon_V05_I_LanternRoom.png)
5. [Fresnel close-up](Saved/VisualRebuildV05/Validation/Captures/BlackBeacon_V05_J_Fresnel.png)
6. [Exterior beam](Saved/VisualRebuildV05/Validation/Captures/BlackBeacon_V05_L_BeaconOn.png)

[Six-view contact sheet](Saved/VisualRebuildV05/Validation/ContactSheet.jpg) ·
[V0.4/V0.5 comparison](Saved/VisualRebuildV05/Validation/BeforeAfter.jpg).
Comparison pairs match camera positions/FOV; moving storm conditions differ.
Fifteen A–O captures, including beacon off and wreck reveal, remain available.
Evidence lives in ignored `Saved/`; source and final Unreal assets are committed.

## Remaining visual weaknesses / next milestone

No known blocker remains in the tested playable route. Owner art acceptance is
separate from this verified playable checkpoint.

- Lens courses are still conspicuously regular; optical refraction and near-axis
  scattering are approximations, and close views retain bright glass highlights.
- Mechanical castings/instruments remain simplified. Some engine faces and the
  lower lantern mechanism are dark; gauges have ticks rather than authored labels.
- Wall weathering can still look procedural/blocky, with repetitive courses and
  occasional strong practical pools. The tower's overall silhouette is inherited.
- Immediate rocks, shoreline contact foam/spray and wreck geometry remain the
  weaker V0.4 assets. No island or coastline geometry rebuild was attempted.
- Performance needs a sustained target-hardware pass beyond the short validation.

Recommended next major milestone: **V0.6 coastal approach and shipwreck hero pass**,
focused on fractured rock formations, convincing sea contact and a substantial
revealed wreck, while preserving the now-validated lighthouse route. Do not start
it automatically or reopen V0.5 as an indefinite material-tweaking session.
