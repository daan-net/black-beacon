# Storm World Identity V0.1 sources

Original deterministic sources, generated with Python's standard library. No downloaded
packs, accounts, paid assets, external libraries, or Niagara plugin are required.

## Regeneration

1. `python3 Art/Source/StormWorld/generate_sources.py`
2. Run `Art/Source/StormWorld/build_assets.py` using the installed Unreal Editor's
   `-run=pythonscript -script=<absolute path> -AllowCommandletRendering` options,
   with `-unattended -vulkan -RenderOffscreen`. Add `-StormReimport` when OBJ/WAV sources
   changed. The builder runs `finalize_assets.py` and writes `Saved/StormReview/AssetAudit.json`.
3. Build BlackBeaconEditor. Run `BlackBeacon.StormWorld.Identity` in the actual game.

The source OBJ meshes use centimetres and an explicit OBJ-to-Unreal handedness conversion (Y and winding are reversed on export). Generated WAVs are mono, 24 kHz, signed 16-bit PCM.
Wind, rain and surf loops crossfade across their loop boundaries. Thunder is a
one-shot. These are synthesized environmental sounds, not field recordings.

## Assets and runtime ownership

- `/Game/BlackBeacon/Storm/Meshes/SM_StormOcean`: 131,072 triangles, finer spacing
  near the island, three directional swells, analytic normals and crest foam.
  No collision, no simulation or navigation. Bounds include 250 cm wave displacement.
- `SM_StormCoast`: finite 8,892-triangle static terrain replaces the old huge square
  ground plane. Existing shore/path/lighthouse route remains at Z=0. Complex-as-simple
  collision is restricted to this static ground surface; ocean and FX do not collide.
- `MI_StormCloudNative`: a project preset of the engine layered volumetric cloud
  material, with weather layout, shape/erosion noise, storm albedo, shared wind
  direction and event-controlled lighting. No panorama is used during play.
- `M_StormOcean`, `M_SeaSpray`, `M_CoastalMist`, `M_RainSplash`, `MI_StormGround`:
  ocean, soft depth-faded coastal FX, and opaque nonmetallic locally damp ground.
- Four `SW_Storm*` assets under `Audio/`: wind, rain, surf and thunder.

`ABBWeatherController` remains the weather authority. Its shared world wind drives
rain position/orientation, cloud advection, spray drift and ocean direction.
`UBBStormPresentationComponent` owns presentation at 20 Hz: native clouds, sea,
154 impact puffs at seven varied sites, seven mist patches and 96 surface splashes.
`UBBStormAudioComponent` mixes the four layers, with positional surf/thunder and
roof-test-driven interior attenuation/low-pass filtering. There is no per-frame
world actor search. `BBStormTiming.h` owns the tested flash/thunder timing.

The old panorama component and old ocean/terrain assets are retained for reversibility;
the panorama and old ocean are hidden during play. No map or gameplay actor is replaced.
The real generator, power, stairs, beam query, objective and reveal systems still run.
Storm begins by default; weather/save restoration still uses the existing controller.

## Review evidence

`BlackBeacon.StormWorld.Identity` generates the A–J views plus a repeated cloud view,
records the actual runtime mix to `Saved/StormReview/BlackBeacon_Storm_RuntimeMix.wav`,
and records a CSV performance sample under `Saved/Profiling/CSV/`. It uses the real
power and beacon APIs. Screenshots alone cannot establish motion/audio quality;
play the route and listen before final visual acceptance.

Art remains V0.1: coast shape, foam and particle cards are deliberately bounded to
the small slice. This is not an ocean simulation or a claim of final production art.

## Tested renderer configuration

UE 5.8.2 / RTX 2060 / NVIDIA 595.91.07 intermittently lost the Vulkan device with
`VK_EXT_descriptor_buffer` during cloud/fog/shadow rendering. The project selects
`r.Vulkan.Bindless.PreferredExtension=1` (`VK_EXT_descriptor_heap`), confirmed by
startup logs. Two consecutive full four-test rendered runs passed with this setting;
native cloud shadows and normal async compute remain enabled. This is a measured
compatibility workaround, not a proven root-cause diagnosis or guarantee for other
drivers. Recheck after an engine/driver change; do not silently remove the setting.

The current checkpoint passed 53/53 logic checks and four Unreal tests twice.
See CURRENT_STATE.md for capture/performance limits and the user review gate.
