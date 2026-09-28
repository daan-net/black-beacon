# Hero Lighthouse Art V0.1 — exterior continuation

This is a new additive pass over V0.5.1 (`06a986f`), not the historical V0.1
pipeline. All eight images under `ART_REFERENCES/LIGHTHOUSE_HERO` informed the
rusticated coastal base, portal, framed windows, cast gallery corbels and peeling
lime finish. The validated tower, gallery/access opening, optical assembly,
interior stairs and generator assets are not regenerated.

Three independent opaque modules, 34,968 triangles total:

- Foundation: eight staggered ashlar courses, chamfered stone faces, coping,
  deep jamb blocks and segmented relieving arch over the original entrance.
- Gallery corbels: sixteen curved closed cast webs, edge flanges, bolted shoes,
  segmented riveted fascia and drip rims beneath the existing gallery.
- Window dressings: twelve projecting hoods, drained sills and stone supports
  aligned to the existing openings.

All modules are in centimetres/Z-up at the tower foot, imported with explicit
normals and Nanite. Runtime attachment has **NoCollision** and navigation disabled.
The V0.5.1 hidden colliders remain authoritative. The original shell's TowerPaint
slot alone receives the new weathered finish; interior, optics, stairs and other
materials are unchanged. Ashlar uses an instance of this isolated material.
No dependencies, plugins, textures or gameplay features are added.

## Editable sources

`generate.py`, `triangulate.py`, `finish.hlsl`, `Generated/*.obj`/`*.mtl` and
`HeroExterior.blend`. Blender 5.0.1 is installed in this environment; the `.blend`
contains the three editable modules in metres, preserving named material slots.
Unreal shading is authored by HLSL; Blender materials are preview placeholders.

```
python3 Art/Source/HeroLighthouseArtV01/generate.py
blender --background --python Art/Source/HeroLighthouseArtV01/build_blend.py
```

Run `import_assets.py` using UnrealEditor-Cmd `-run=pythonscript`, Vulkan offscreen
and `-AllowCommandletRendering`. This imports only these three modules and creates
`M_Hero_ExteriorFinish` plus `MI_Hero_Ashlar`. Do not rerun older blanket importers.
Runtime integration is in `UBBHeroArchitectureComponent`.

Validation and screenshots: `Saved/HeroLighthouseArtV01/` (ignored evidence),
`HERO_LIGHTHOUSE_ART_V01.md` (tracked report). Owner visual/UX acceptance is separate.
