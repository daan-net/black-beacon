# BLACK BEACON — Content

This folder is where authored Unreal content will live. At the time of the
bootstrap milestone there are **no `.uasset` files here on purpose**: the
project is fully runnable from C++ and text (see `../Source` and
`../Config`). `.uasset`/`.umap` files are binary and must be created inside
the Unreal Editor in milestone 0.2+.

## Planned folder layout (create in the editor)

| Folder | Intended content |
|---|---|
| `Content/Environments/` | Authored levels: `Slice_01.umap` (coastal bay + lighthouse) |
| `Content/Environments/Materials/` | PBR island/lighthouse materials, wetness helpers |
| `Content/Props/` | Modular lighthouse set, island rocks kit, keeper's props |
| `Content/Niagara/` | Rain, mist, beam-assisted moisture, generator exhaust |
| `Content/UI/` | Objective HUD (C++ Slate in 0.1; polished UMG later) |
| `Content/Audio/` | Wind, rain, ocean, machinery, beam signature, reveal cue |
| `Content/Blueprints/` | Only content-specific glue wrapping C++ systems (if ever) |

## Rules

- Do not commit work-in-progress asset noise. Each asset must earn its place.
- Gameplay systems never depend on assets: actors/components query meshes by
  reference set in the editor, they do not *require* specific assets to exist.
- The greybox slice (built by `UBBProceduralWorld`) is dev tooling and is
  replaced by `Slice_01.umap`; keep the builder in the project for tests.