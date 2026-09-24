# Hero lighthouse V0.1 implementation record

## Reference set

The eight PNGs in this directory are the visual source of truth. They show the
storm coast hero composition, four exterior elevations, and close views of the
lantern, gallery, entry, windows, rock base, and scale. They are supplied
reference art and are not project-authored game assets.

- `ChatGPT Image Sep 24, 2026, 09_55_58 AM (1).png`
- `ChatGPT Image Sep 24, 2026, 09_55_58 AM (2).png`
- `ChatGPT Image Sep 24, 2026, 09_55_58 AM (3).png`
- `ChatGPT Image Sep 24, 2026, 09_55_58 AM (4).png`
- `ChatGPT Image Sep 24, 2026, 09_56_16 AM (1).png`
- `ChatGPT Image Sep 24, 2026, 09_56_17 AM (2).png`
- `ChatGPT Image Sep 24, 2026, 09_56_17 AM (3).png`
- `ChatGPT Image Sep 24, 2026, 09_56_17 AM (4).png`

## Source and engine asset layout

- `Art/Source/Lighthouse_Hero/generate_lighthouse_obj.py` deterministically
  generates the modular OBJ source meshes with Python's standard library.
- `Art/Source/Lighthouse_Hero/import_lighthouse_meshes.py` imports OBJ modules
  through UE 5.8.2 Interchange and moves imported material assets into the
  project's lighthouse material directory.
- `Art/Lighthouse/SourceMeshes/` stores the generated OBJ/MTL source pairs.
- `Content/BlackBeacon/Art/Lighthouse/Meshes/` stores the imported tower shell,
  gallery, lantern room, annex details, and visual rock plinth.
- `Content/BlackBeacon/Art/Lighthouse/Materials/` stores the six OBJ material
  families: tower paint, dark iron, warm brass, lantern glass, aged wood, and
  wet rock.

Blender was not installed on the development machine. These deterministic OBJ
sources were used instead; no `.blend` source is claimed. The source generator
uses centimeters, Z up, and a tower-foot origin. The lantern optical pivot is
at tower-relative Z 1980 cm, coincident with the current lighthouse controller
and beam component origin.

## Gameplay integration

The existing `ABBLighthouseController`, `UBBLighthouseBeamComponent`, reveal
query, generator component, objective actors, and stair actors remain the
gameplay authority. Imported hero meshes are visual-only and non-colliding.
The tower's existing paint material instance is retained on the new shell. The
lantern room mesh is aligned to the controller's beam origin; the existing beam
and its powered visibility state remain in place.

The art pass relocates the existing functional generator shed and its objective
trigger to the keeper annex beside the tower. It keeps their actors, interaction
components, and blocking shell. Stair tread count and rise remain 28 per floor
and 520 cm per floor; the three flights now fit the tapered tower better, with
their radius/depth changed to 190/130, 155/110, and 130/90 cm. Saved-map stair
collision remains authoritative. The separate hero tower mesh has no collision.

The visual foundation is a low-poly contact ring, not a replacement for the
island terrain. Annex facade details sit over the existing traversable shell.
Windows are surface details and are not cut-through openings. The interior
stairwell keeps the existing functional stair geometry and lighting; it is not
yet a finished architectural interior.

## Material correction record and limits

UE's initial OBJ/MTL import created all six `M_LH_*` instances under an
Interchange-generated Phong master. The MTL `Ks`/`Ns` values were high for
stone, dark iron, and glass, causing metallic-looking specular highlights even
where the material names implied rough painted surfaces. The instances now use
the project's opaque PBR masters with explicit Metallic, Roughness, Specular,
and WetAmount parameters:

| Material family | Metallic | Dry roughness | Notes |
|---|---:|---:|---|
| Tower paint | 0.00 | 0.84 | Opaque; weathered whitewash |
| Dark iron | 0.12 | 0.72 | Oxidized stair/gallery metal |
| Warm brass | 0.82 | 0.43 | Restrained mechanical wear |
| Lantern glass family | 0.00 | 0.58 | Opaque dark tower windows |
| Aged wood | 0.00 | 0.76 | Matte wood |
| Wet rock | 0.00 | 0.62 | Non-metallic basalt |

The actual Fresnel lens highlight uses the separate additive, unlit
`M_LanternLens`; opaque tower, stair, door, and rock surfaces do not use a glass
blend mode. The audit found the two used surface textures are albedo-only, with
sRGB enabled, default color compression, and world mip settings. No ORM, normal,
roughness, metallic, or AO maps are present, so no packed channels could be
misassigned. The material has no clearcoat. A procedural world-normal and
object-height mask blends localized wet roughness and darkening; it does not
modify Metallic. The tower shell uses cylindrical UVs with approximately 2 m
tiling, and generated box faces use UV scale based on face dimensions.

These are material response and UV corrections, not finished texture authoring.
There are no dedicated authored normal/roughness/metallic/AO/height maps. Close
interior views still show pale walls under local lights, and the lantern-room
review capture is dark. The exposure and light placement need further human
review. There is no authored LOD chain. Mesh triangle counts are modest for
this hero asset, but the storm scene still needs a representative performance
profile on the RTX 2060 before visual optimization is considered complete.

## Current validation record

The rendered GameplayFlow automation exercises generator startup, power,
stair traversal assertions, beacon state, beam/reveal progression, and captures
the in-engine views under `Saved/Screenshots/LinuxEditor/`. The four dedicated
hero views are named:

- `BlackBeacon_Hero_A_ExteriorThreeQuarter.png`
- `BlackBeacon_Hero_B_AnnexEntrance.png`
- `BlackBeacon_Hero_C_Stairwell.png`
- `BlackBeacon_Hero_D_LanternRoom.png`

The existing suite also captures beacon on/off, storm, and reveal states. These
are real UE renders, not concept art. They are an integration and gameplay
checkpoint, not evidence that the target production art quality has been
reached. The stair frame is captured from the player during the real traversal
test; the lantern frame is captured after the normal interaction powers the
Fresnel elements. The current frames show that stairwell composition and close
lens exposure still need a visual review. Inspect the renders before accepting
the next art direction pass.

The material correction pass additionally captures:

- `BlackBeacon_Material_01_StairwellUp.png`
- `BlackBeacon_Material_02_ExteriorUp.png`
- `BlackBeacon_Material_03_BaseRocks.png`
- `BlackBeacon_Material_04_StairClose.png`
- `BlackBeacon_Material_05_LanternRoom.png`

The full UE 5.8.2 Vulkan M01 automation suite passed 3/3 on 2026-09-24, and
`./Tools/validate.sh` passed all 45 engine-independent checks. These captures
support that the reparented materials load and render, but the close interior
brightness and lantern-room darkness remain open review items.
