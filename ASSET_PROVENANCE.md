# Asset provenance

## Coastal Boulder Mesh

- Source: UE 5.8.2 installed engine PCG sample content,
  `/Engine/Plugins/PCG/Content/SampleContent/SimpleForest/Meshes/PCG_Boulder_02.uasset`.
- Referenced as `/PCG/SampleContent/SimpleForest/Meshes/PCG_Boulder_02` and
  instanced with the project's wet-basalt material. No PCG gameplay module or
  graph is used by BLACK BEACON.
- This is engine sample content, not an external or purchased asset. The coast
  field remains placeholder geometry pending authored terrain assets.

## Shipwreck Hull Section

- Source: [3DAssets.dev — Shipwreck Hull Section](https://3dassets.dev/assets/sunken-city-and-underwater-ruins-shipwreck-hull-sectio-bdebf831)
- Original file: `Assets/External/shipwreck-hull-section.glb`
- Imported into Unreal with the UE 5.8.2 built-in glTF/Interchange importer.
- Listed dimensions: 8.594 × 3.115 × 3.889 m; 952 triangles; 4 materials.
- Licence: CC0 1.0 Universal, as listed on the asset page. The page reports that AI was used to create the model.
- Local SHA-256: `4031cbdd9ef62ae567fc02b33ae5ffa775c05a069ea0150d9a90854963066f98`

## Wreck Hull Albedo

- `Content/BlackBeacon/Textures/T_WreckHullAlbedo.png` was generated for this project with OpenAI ImageGen on 2026-09-23.
- Local SHA-256: `614fb0cb1cf7f2b18b783e5c20ba2905aa8341229cc54d31b621fa2754a7e264`

## Lighthouse Paint Albedo

- `Content/BlackBeacon/Textures/T_LighthousePaintAlbedo.png` was generated for this project with OpenAI ImageGen on 2026-09-23 as a weathered, north Atlantic lighthouse whitewash surface.
- Local SHA-256: `d2705625ed889f4992e9f318018c4650a14b40454b9be7aa6fff2c7f612fbe57`
- Imported as `T_LighthousePaintAlbedo.uasset` and applied to the existing non-colliding lighthouse exterior skin.

## Storm World Identity V0.1 (2026-09-24)

- `Art/Source/StormWorld/generate_sources.py` creates the original ocean/coast meshes
  and four original synthesized WAV layers using only the Python standard library.
  Reproducible source outputs are retained in `Generated/`.
- `build_assets.py` imports them into `/Game/BlackBeacon/Storm` and builds the native
  cloud, ocean, spray, mist, splash and damp-ground materials. No external download,
  paid service or third-party package was introduced.
- The cloud preset derives from engine-bundled
  `/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst` and its volume,
  weather and erosion textures; subject to the existing
  Unreal Engine content licence, not a newly sourced CC0 texture.
- Audio is synthesized wind/rain/surf/thunder, not recorded location audio. Final
  sound-design quality requires listening review. `AudioMixer` is an existing engine
  module used to capture the real runtime mix during automation; no new plugin.
