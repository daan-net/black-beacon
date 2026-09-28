# Black Beacon environment acquisition plan

2026-09-28 · branch `work/asset-first-environment-pass` · baseline `3216c5c`.
Original storm-bound North Atlantic industrial lighthouse. Cinematic survival-game
references inform composition and material quality only; no commercial-game content
is copied. Preserve the lighthouse's identity and all gameplay geometry.

## Audit before acquisition

The current checkout contains 78 project `.uasset` files (including legacy assets):
25 lighthouse mesh/import packages, 19 lighthouse materials, 9 general materials,
6 storm materials, 2 storm meshes, 4 storm sounds, 4 imported wreck meshes and
4 wreck materials, 4 textures and one rain effect. No acquired Fab/Megascans pack
is present in this checkout. Experimental branches were not merged.

| Existing group | Sources / ownership | Decision |
| --- | --- | --- |
| Tapered tower, portal, gallery, lantern cage | `Art/Source/VisualRebuildV04`, `Art/Lighthouse/SourceMeshes` | REUSE CURRENT. Dimensions define the playable route; visual shell stays separate from collision. |
| Foundation ashlar, gallery corbels, window dressings | `Art/Source/HeroLighthouseArtV01/HeroExterior.blend`, Python/OBJ/HLSL | REUSE CURRENT. These carry lighthouse identity and improve silhouette. |
| Fresnel rotor, arc source, engine, alternator, flywheel, stairs | `Art/Source/HeroEnvironmentV05` Python/OBJ | REUSE CURRENT mechanism and interaction composition; MODIFY EXISTING materials only when warranted. |
| Lantern service door, gallery hatch/deck | `Art/Source/PlaytestV051` Python/OBJ | REUSE CURRENT. Gameplay-critical access; no dimensional changes. |
| Generic procedural coastal rocks/plinth | V0.4 Python meshes | DOWNLOAD / ACQUIRE scans for visible foreground; retain originals and collision. |
| Painted walls, plaster, stone, metal, wood | `M_V04_WeatheredSurface`, V0.5 surface HLSL, `M_Hero_ExteriorFinish`, `M_LH_*` | MODIFY EXISTING via isolated overrides. Acquire scanned PBR wall/stone/metal; procedural micro-noise is not a replacement for captured surface detail. |
| Ocean, terrain, moving clouds, rain/spray, storm audio | `Art/Source/StormWorld`, weather presentation components | REUSE CURRENT. Keep weather authority, rendering configuration and shoreline collision. |
| Shipwreck reveal hull | Four imported hull sections; historical root provenance file | REUSE CURRENT target/reveal. Future unique silhouette and story salvage require a separate pass. |
| Generic annex pipes, lamps, boxes, furniture/clutter | Existing annex module and generator visuals | Keep working machinery; DOWNLOAD / ACQUIRE supporting service equipment instead of new primitive modeling. |

The only existing `.blend` under `Art/` is `HeroExterior.blend`; the earlier
V0.4/V0.5 geometry is scripted OBJ source, not hidden Blender projects. Preserve
all sources. The existing storm panorama is legacy, not the active dynamic sky.

## Category decisions

| Category | Classification | Implementation / acquisition direction |
| --- | --- | --- |
| A. Lighthouse exterior | REUSE CURRENT + MODIFY EXISTING | Keep silhouette, ashlar and portals; scanned plaster over the existing shell, localized runoff and damp footing. |
| B. Lighthouse interior | REUSE CURRENT + MODIFY EXISTING | Preserve stair collision, landings and clearance; reuse scanned plaster with a dry, warmer material instance. |
| C. Lantern room | REUSE CURRENT; CUSTOM BUILD only for identity | Retain bespoke Fresnel, bearing, cage and controls. No generic replacement or new mechanism this pass. |
| D. Generator annex | MODIFY EXISTING + DOWNLOAD / ACQUIRE | Preserve engine/control interaction; scanned wall finish and a small fuel-storage grouping outside the walking corridor. |
| E. Shoreline | DOWNLOAD / ACQUIRE | Coastal ledge scans at immediate lighthouse footing and water contact; existing ocean/spray retained. |
| F. Cliffs / rocks | DOWNLOAD / ACQUIRE | Layered scanned rock faces; no island rebuild. Fab Nordic Coastal Cliff is a future alternative, not an entitlement assumption. |
| G. Vegetation | DOWNLOAD / ACQUIRE | Low coastal grasses, moss and algae only in sheltered wet cracks; no forest kit or new biome. Deferred. |
| H. Industrial props | DOWNLOAD / ACQUIRE | Fuel drums now; pipes/valves and tools next, arranged as maintenance evidence. |
| I. Electrical props | REUSE CURRENT + DOWNLOAD / ACQUIRE | Keep functional switchboards; acquire boxes, conduits and junctions when a clearly licensed set is selected. |
| J. Weather / wetness | REUSE CURRENT + MODIFY EXISTING | Keep cloud/fog/rain presets; shared material wetness controls with distinct sheltered and sea-contact settings. |
| K. Shipwreck area | REUSE CURRENT; CUSTOM BUILD only for unique pieces | Keep reveal targets and location; acquire generic timber/salvage later. Unique wreck history is custom scope. |
| L. Environmental storytelling props | DOWNLOAD / ACQUIRE + CUSTOM BUILD | Generic maintenance storage/tools acquired; bespoke keeper logs, notices and incident clues authored only when story needs them. |

## First acquisition batch — six entries maximum

Prices/status checked 2026-09-28; no paid purchase is authorized or made.

| Exact asset | Source | Cost / access | Why / disposition |
| --- | --- | --- | --- |
| Painted Plaster Wall — Amal Kumar | [Poly Haven](https://polyhaven.com/a/painted_plaster_wall) | Free CC0; no account | 2 m PBR scan for exterior, sheltered interior and annex. Acquire now. |
| Coast Rocks 05 — Rob Tuytel | [Poly Haven](https://polyhaven.com/a/coast_rocks_05) | Free CC0; no account | Eroded 3 m coastal ledge and embedded debris; immediate footing. Acquire now. |
| Rock Face 02 — Dario Barresi / Rico Cilliers | [Poly Haven](https://polyhaven.com/a/rock_face_02) | Free CC0; no account | Layered rock faces and fissures; near-coast silhouette. Acquire now. |
| Barrel 03 — Serhii Khromov | [Poly Haven](https://polyhaven.com/a/barrel_03) | Free CC0; no account | Worn fuel drum with captured paint/metal separation; annex context. Acquire now. |
| Nordic Coastal Cliff — Quixel Megascans | [Fab](https://www.fab.com/listings/35d695d8-ac00-4ac5-8bba-1d46e973af68) | Listed Free; library/account entitlement and license confirmation required | Optional coastal alternative. Not downloaded; do not substitute scratch geometry if acquisition is blocked. |
| Modular Industrial Pipes 01 | [Poly Haven](https://polyhaven.com/a/modular_industrial_pipes_01) | Free CC0; no account | Next generic service kit after this bounded pass, not needed to complete current integration. |

Fab is preferred when the asset and entitlement are available; this checkout has
no authenticated acquisition workflow. The first integration uses four documented
CC0 assets instead. No asset with unclear license enters the project. Fab source
assets must not be committed to the public repository unless their actual license
allows source redistribution; cooked-game permission alone is insufficient.

## Focused pass and stop condition

Acquire four assets, create the reusable kit materials/meshes, override only visual
wall slots, add noncolliding coastal skins and restrained fuel-storage props.
No new map, generic scratch modeling, gameplay code, collision, sky or global
renderer changes. Targeted editor build and a rendered review with eight views and essential
route/interaction checks; correct concrete import/shader failures only, record
limitations, commit and stop.
