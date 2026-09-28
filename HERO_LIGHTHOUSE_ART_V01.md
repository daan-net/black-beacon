# Hero Lighthouse Art V0.1 — exterior continuation

2026-09-28, `work/v051-playtest-candidate`, based on `06a986f`.
This is the owner's approved post-recovery exterior continuation, not a rerun
of the historical V0.1 generator and not the V0.6 coastline milestone.

## Recovery and safety

The recovery found a clean working tree at `06a986f`. The first `BB_StairGuard`
correction and V0.5.1/M01 passing evidence were already committed. No door-leaf
experiment or partial visual source survived in the tree. The requested
`checkpoint-v051-playtest-candidate-20260926` now preserves that technical baseline.
Existing V0.4/V0.5 tags were not moved. See `V051_VALIDATION.md` for collision evidence
and exact owner controls. Owner manual UX acceptance remains pending.

## Visible work

- Eight courses of separately modeled, staggered and chamfered ashlar around
  the lower tower; real bed joints, coping, deep portal blocks and a segmented
  relieving arch over the working doorway.
- Sixteen curved cast gallery brackets with closed webs, edge flanges, bolted
  shoes and a riveted segmented fascia. The gallery support silhouette is deeper.
- Projecting hood moulds, sills and stone supports on all twelve existing windows.
- Isolated world-space exterior paint loss, exposed substrate, runoff and lower
  damp zones. The old repeated texture squares are removed from the tower paint
  slot. Interior materials and lighting are unchanged.

Three new opaque Nanite meshes: `SM_BB_Hero_Foundation`,
`SM_BB_Hero_GalleryCorbels`, `SM_BB_Hero_WindowDressings` (34,968 source triangles).
Two new Unreal material assets: `M_Hero_ExteriorFinish`, `MI_Hero_Ashlar`.
The existing lighthouse meshes, access assets, gameplay collisions and all
reference PNGs remain unchanged. New components have collision/navigation disabled.

Editable source: `Art/Source/HeroLighthouseArtV01/` contains Python, OBJ/MTL,
HLSL and `HeroExterior.blend` (Blender 5.0.1, three additive modules in metres).
Blender materials are preview placeholders; Unreal owns the actual finish.

## Validation

**IMPLEMENTED / TESTED technical art candidate.** Editor build, scoped Unreal
import and `Tools/validate.sh` pass (logic tests plus seven geometric checks,
including the visible portal opening and exterior/stairwell clearance).

- `BlackBeacon.V051.PlaytestLoop`: Success, 39.71s, NullRHI, exit 0. Dynamic
  doorway approaches finish at X=254.851 / 252.234 / 241.309 within the unchanged
  0.6s deadline. Full climb, balcony, generator, yaw/pitch, release/reacquire,
  manual wreck search/discovery and objectives pass.
- `BlackBeacon.M01.GameplayFlow`: Success, NullRHI, exit 0. The first run found
  only an obsolete `M_LH_` prefix assertion for the tower's first material. Its
  replacement verifies the exact exterior finish and equality of every other
  slot with the imported asset. No gameplay expectation was weakened.
- `BlackBeacon.HeroExterior.Review`: Success, 33.30s, Vulkan native 1920x1080,
  exit 0, zero test warnings/errors. All three imported modules resolve, use
  tower-foot transforms and have no collision. Five actual game-world views.

The first visual review prompted one finish correction: reduce broad dark
camouflage-like patches to finer, lower-contrast peeling. Geometry needed no
correction. The final finish was inspected in Vulkan; the independent NullRHI
playtest ran during that material-only compilation. No shader/render test is
claimed from NullRHI. Initial failed/review logs are retained separately.

Evidence: `Saved/HeroLighthouseArtV01/` contains Build.log, Import.log,
FinishCorrection.log, LogicValidation.log, Playtest.log/PlaytestReport,
RegressionFinal.log/RegressionReportFinal and VisualReviewFinal.log/VisualReportFinal.

## Actual captures

[Before/after slider](Saved/HeroLighthouseArtV01/Comparison.html) uses the same
camera/FOV as the retained V0.5 three-quarter capture; rain/cloud/beam phase differs.
No image retouching or exposure correction was applied. These are automated
review cameras in the actual game world, not manual player screenshots.

- [Full lighthouse](Saved/HeroLighthouseArtV01/Captures/BlackBeacon_HeroExterior_A_FullLighthouse.png)
- [Hero three-quarter](Saved/HeroLighthouseArtV01/Captures/BlackBeacon_HeroExterior_B_HeroThreeQuarter.png)
- [Base, inherited camera](Saved/HeroLighthouseArtV01/Captures/BlackBeacon_HeroExterior_C_Base.png)
- [Gallery structure](Saved/HeroLighthouseArtV01/Captures/BlackBeacon_HeroExterior_D_GalleryStructure.png)
- [Frontal entrance](Saved/HeroLighthouseArtV01/Captures/BlackBeacon_HeroExterior_E_Doorway.png)

Evidence remains ignored according to repository hygiene. The source assets,
Unreal assets, integration, tests and this report belong to the art commit.
Owner visual approval and physical-mouse playtesting remain outstanding.

## Limits

The pre-existing shaft bands, lantern cage/roof, optics and annex are retained;
this does not claim the entire lighthouse now matches the reference art exactly.
The storm lighting still hides some underside detail. Weathering is procedural,
without individually painted damage or scanned stone. The inherited base review
camera is partly occluded by a rock; a new frontal doorway view is supplied.
No coast/wreck hero work, packaging or unrelated mechanics were started.
