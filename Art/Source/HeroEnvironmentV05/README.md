# V0.5 hero environment sources

Approved scope: lantern architecture, Fresnel optics, machinery and material
readability on the validated V0.4 checkpoint. No experimental branch assets,
external dependencies or gameplay features are used. All eight approved images
in `ART_REFERENCES/LIGHTHOUSE_HERO/` informed the cast framing, copper dome,
prism drum, brass roller mechanism, switchboards and coastal weathering.

Blender is not installed. Editable source is `generate.py`, reusing the small
V0.4 mesh library. `Generated/` contains six OBJ/MTL modules and `MeshAudit.json`
records triangle counts. Units are centimetres; OBJ export explicitly reverses
handedness. `export_obj.py` supplies explicit welded smooth normals because
Unreal Interchange ignores OBJ smoothing groups. Materials use world mapping
rather than stretched primitive UVs.

```
python3 Art/Source/HeroEnvironmentV05/generate.py
```

Run `build_assets.py` inside the installed Unreal Editor Python commandlet with
Vulkan, offscreen rendering and `-AllowCommandletRendering`. It compiles only
the affected surfaces/optics/shaft and imports only the six generated modules.
The older V0.4 blanket build must not be used to reproduce this milestone.
Persistent destinations are `/Game/BlackBeacon/Art/Lighthouse/{Meshes,Materials}`
and the existing `/Game/BlackBeacon/Materials/M_BeamShaft`.

The V0.5 stairs deliberately adopt the previously source-only V0.4 maintenance
lamps and retain all 84 exact tread heights/footprints. Lantern source is replaced
entirely. The annex mesh and its unexported roof variation are left alone. The
validated gallery/deck files are not regenerated; the 200-degree hatch remains.

Runtime presentation applies after saved-map adapters. The generator's original
collider remains authoritative; visual meshes use absolute unit scale to avoid
inheriting its block scale. Its original flywheel pivot/timer drive the new open
spoked wheel. The optical rotor follows the existing beam pivot in azimuth; its event-driven
pitch compensation keeps the carriage on its level bearing. The small
arc mesh inherits the existing power-driven source visibility. All new visuals
have no collision. The instrument console occupies the original control volume.

`surface.hlsl` defines macro wear, runoff and salt, cast enamel, brass oxidation,
sheltered masonry and upward-facing tread pattern. `beam.hlsl` defines finite
ray-integrated mist with soft radial density and axial extinction. The native
spotlight remains enabled at reduced volumetric scattering; gameplay query range,
angle, rotation and reveal remain unchanged. Dynamic clouds/rain/sea are preserved.

Evidence is in ignored `Saved/VisualRebuildV05/`; see `V05_VALIDATION.md` for actual
build/test status. Do not infer visual acceptance from asset generation alone.

Opaque generator/flywheel/stair modules use Nanite to avoid the observed VSM
non-Nanite marking overflow. Glazing, optical glass and bulb sections do not cast
opaque shadows; structural metal retains shadows. Commandlets on UE 5.8 do not
instantiate StaticMeshEditorSubsystem, so the build uses a transient fallback
for its stateless section methods (currently emits a deprecation notice).
