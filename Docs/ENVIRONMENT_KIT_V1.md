# BLACK BEACON ENVIRONMENT KIT v1

Reusable asset-first foundation; imported into `/Game/BlackBeacon/EnvironmentKitV1`.
Status before import/build: PLANNED integration, acquisition in progress.

| Reusable category | v1 disposition |
| --- | --- |
| Wet rock / cliff set | CC0 Coast Rocks 05 and Rock Face 02; source UV PBR instances, localized dampness. |
| Concrete / weathered paint | Scanned Painted Plaster Wall; world-aligned master at a documented 200 cm repeat, exterior and sheltered instances. |
| Rusted metal / old wood | Existing `M_LH_DarkIron`, `M_LH_AgedWood` retained; dedicated scans deferred. |
| Moss / algae / grime / leaks | Captured rock coloration plus localized wall runoff in shared material; reusable decal acquisition deferred. |
| Generic debris / industrial clutter | Scanned fuel drum now; tools and debris deferred. |
| Pipes / conduits / electrical boxes | Existing annex/control equipment retained; modular pipe acquisition queued. |
| Lamps / doors / windows / railings | Reuse current lighthouse architecture and warm practical fixtures; no traversal changes. |
| Vegetation | Deferred coastal grass kit; do not scatter temperate forest assets. |
| Fog / weather presets | Reuse current native storm/cloud, ocean, rain, spray and warm practical settings. No global renderer change. |

Normalization: Unreal centimetres; 2K PBR first batch, base color sRGB, masks and
DirectX normals linear. Respect source dimensions and UVs; avoid stretched props.
World-aligned architectural scans use physical repeat size. A shared material
controls tint, saturation, roughness and wetness. Shelter stays rougher than sea
contact; no universal glossy coating. Retain directional grain/strata orientation.

Opaque coastal meshes use Nanite when beneficial, source UVs and existing Lumen
settings; small props use normal static mesh rendering. No new plugin, rendering
feature or engine dependency. Collision/navigation is disabled for new visual
components. Existing movement, stairs, hatch, generator, controls, reveal and
objective actors remain authoritative. No legacy assets are deleted.

Reproduce acquisition with `python3 Art/Source/EnvironmentKitV1/acquire.py`.
Import with `import_assets.py` in the existing Unreal Python commandlet.
The scene assembly is isolated from gameplay and can be removed without changing
route dimensions. Final validation evidence will be recorded here after rendering.
