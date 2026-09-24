# Major visual rebuild V0.4

Presentation scope authorized 2026-09-25. All eight supplied lighthouse images
were reviewed as a package: battered tapered masonry, deep framed openings,
bracketed gallery, glazed metal lantern, domed metal roof, coastal service annex,
and contrasting warm practicals against the storm. No new gameplay mechanic,
plugin, external library or downloaded asset is introduced.

## Reproducible source

Blender is absent on this host. `mesh.py` and `generate.py` are the editable,
deterministic Python source; no `.blend` file is claimed. Centimetres, Z up,
explicit OBJ handedness conversion, grouped material sections, face-scaled UVs.
Generated OBJ/MTL files live in `Art/Lighthouse/SourceMeshes/`.

1. `python3 Art/Source/VisualRebuildV04/generate.py`
2. `python3 Tests/test_visual_sources.py`
3. Run the installed UnrealEditor with the absolute project path,
   `-run=pythonscript -script=<absolute>/Art/Source/VisualRebuildV04/build_assets.py`
   and `-unattended -vulkan -RenderOffscreen -AllowCommandletRendering`.
4. Build BlackBeaconEditor Linux Development, then `./Tools/validate.sh`.
5. `./Tools/review_v04.sh` runs the integrated regression suite and A–O captures.

`MeshAudit.json` records source triangle counts. Architectural meshes retain
separate material families; the world-space surface shader avoids dependence
on scaled primitive UVs. Weathered whitewash, sheltered plaster, oxidized iron,
brass, wood and basalt use distinct roughness, metallic and dampness parameters.
The former imported Phong materials are not used by the rebuilt meshes.

## Gameplay seam and collision

The existing generator, objectives, player movement, 84 stair actors, controller,
rotation, power and `FBBBeamQuery` remain authoritative. The visual tread tops
match the existing 520/28 cm rise exactly. Visual stair/core/guard geometry replaces
rendering only; the original collision components remain active and hidden.
The same applies to annex shell and coast boulder collision.

One supplemental collision mesh supplies the gallery walking surface around the
last-flight hatch. The original final landing is retained. Automated probes
check three floor locations and the open hatch; the existing traversal test
walks every stair rise. Full-height access doors and recessed window openings
are modeled, rather than drawn onto a solid tower.

`UBBHeroArchitectureComponent` performs a single post-BeginPlay migration after
the existing saved-map adapters. The Fresnel rotor attaches to `BeamVisualPivot`;
it adds no simulation timer or alternate beam logic. Architectural fixtures
and existing floor lights provide localized warm pools.

## Atmosphere and optical presentation

Only the weather controller's main moon directional light remains. Lightning
adds the timed flash to its configured moon baseline, while cloud emission and
thunder use the existing storm envelope. Nothing suppresses light warnings.

The installed UE 5.8 renderer source was checked for console variable names:
`VolumetricRenderTarget.cpp`, `VolumetricFog.cpp`, `VolumetricCloudRendering.cpp`.
Cloud target mode 1 traces at half resolution instead of quarter resolution.
View sample scale is 2.0; cloud shadow map scale and shadow view sampling are 1.0.
Fog uses a 6 pixel XY grid, 128 depth slices, reprojection with 0.9 history weight,
and 8 samples when history is missing. Native-resolution TAA is explicit;
TSR and dynamic resolution are not layered over it. This raises GPU cost and
requires rendered performance review on the target hardware.

Supporting engine documentation:
- https://dev.epicgames.com/documentation/unreal-engine/volumetric-cloud-component-properties-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/volumetric-fog-in-unreal-engine

The beam retains its physical spotlight and gameplay cone. A 12-sample camera-ray
integration through the cone provides additive scattering with a cubic radial
falloff, distance extinction and slowly varying mist. It cannot produce opaque
coverage. Lantern glazing stays transparent in both power states; the lens and
Fresnel bands retain their existing power-driven visibility.

Ocean geometry and wind remain owned by the storm system. Additional crossing
short waves break the swell regularity; whitecaps and foam are concentrated
around the existing seven impact/spray sites.

## Recovery and evidence

`checkpoint/pre-v04-20260925` identifies source commit `7bc4c0a`.
`Saved/Checkpoints/Before_VisualRebuild_V04.tar.gz` preserves the complete initial
working tree, including the two pre-existing texture modifications and references.
It excludes generated engine state. Do not extract over newer user changes;
restore into a separate directory when reviewing the prior version.

Prior screenshots are copied under `Saved/VisualRebuildV04/Before/`.
New real Unreal screenshots use `BlackBeacon_V04_A_` through `BlackBeacon_V04_O_`
under `Saved/Screenshots/LinuxEditor/`. Evidence, build logs and automated reports
stay under ignored `Saved/`; they are not source assets. See CURRENT_STATE.md for
actual results and remaining acceptance limits.
