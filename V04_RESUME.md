# V0.4 resume checkpoint

User paused on 2026-09-25 to wait for usage-limit reset. Keep the main branch
playable on the previous verified version. This branch saves unfinished work.

## State

- Original source: `7bc4c0a`, tag `checkpoint/pre-v04-20260925`.
- Full original working tree, including user modifications:
  `Saved/Checkpoints/Before_VisualRebuild_V04.tar.gz`.
- Before screenshots: `Saved/VisualRebuildV04/Before/`.
- All eight supplied reference images were reviewed. Blender is absent;
  Python OBJ source generation uses no added dependency.
- Pass A–E source changes exist. UE editor C++ target builds successfully.
- `./Tools/validate.sh` passes 53 logic checks + four architectural checks.
- `build_assets.py` succeeded with null RHI (first asset import).
- Second import with Vulkan spent several minutes compiling shaders and was
  interrupted at the user's pause. Its generated meshes/materials are incomplete
  as an integrated set; do not play or accept this branch without finishing.
- No integrated V0.4 gameplay run or A–O evidence has happened.

## Resume in this order

1. Switch to `wip/v04-visual-rebuild-20260925` (preserve new user edits first).
2. Read AGENTS.md and `Art/Source/VisualRebuildV04/README.md`.
3. Run `python3 Art/Source/VisualRebuildV04/generate.py`. Recent source fixes
   (roof winding, degenerate triangles, lamp fixtures) have NOT been regenerated.
4. Run `build_assets.py` in Unreal with the ABSOLUTE project path, Vulkan,
   RenderOffscreen and AllowCommandletRendering. Material compilation may take
   several minutes on the i5-8400; shader worker input timestamps showed progress.
   Do not use a relative project path (engine changes cwd).
5. Run `import_meshes.py` in UE to audit <=16 material sections per mesh and
   ensure transparent window material bindings and gallery complex collision.
6. Build BlackBeaconEditor, run `./Tools/validate.sh`, then `Tools/review_v04.sh`.
   This is the intended ONE integrated gameplay validation after all passes.
7. Inspect all real A–O captures and the previous matching exterior screenshot.
   Correct genuine integration/visual failures as necessary; no tiny tuning loop.
8. Record results honestly in CURRENT_STATE.md / CHANGELOG.md; only then merge
   accepted work to main and make the milestone delivery.

## Important technical details

- `UBBHeroArchitectureComponent` applies saved-map visual migration once, after
  existing actors' BeginPlay. Keeps the 84 original stair collision actors.
- A supplemental gallery deck supplies floor collision around the stair hatch.
- Real tapered wall openings, hollow gallery rings, rebuilt full-height lantern,
  rotating Fresnel, structural stair art, fitted annex, shaped coast boulders.
- Main directional light now supplies lightning; removed competing second light.
- Higher cloud/fog quality and explicit TAA; additive integrated beam density.
- New material family uses world-space triplanar texture scale and local wetness.
- Imported temporary OBJ material assets under the Meshes directory are unused
  after slot reassignment. They are not intended deliverables.
- New C++ test `BlackBeacon.V04.ArchitectureReview` captures A–O and checks the
  directional light count, rotor attachment, deck supports and hatch clearance.
- Existing GameplayFlow assertion was changed from old dynamic material identity
  to imported architectural slots; gameplay/reveal tests are otherwise retained.

## User files

Two texture files were already modified before work, and eight reference PNGs
were untracked. Preserve these exactly; do not include them in the V0.4 commit:
`T_StormSkyPanorama.uasset`, `T_WreckHullAlbedo.uasset`, reference PNGs.

Build/log evidence is under ignored `Saved/VisualRebuildV04/`.
