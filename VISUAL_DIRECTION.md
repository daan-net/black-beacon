# BLACK BEACON — Visual Direction

Premium, cinematic, physically believable. Visual quality is a product feature — but
never confused with feature count. Few things, finished beautifully.

## 1. Master palette & lighting

- Light: cold North Atlantic daylight in storms (blue-grey), warm tungsten inside the
  lighthouse (lantern, keeper's quarters), and the beam itself — warm white/amber
  cutting through fog and rain.
- Filmic exposure: UE5 Auto Exposure with cinematic min/max EVs so coming out of the
  dark annex into the beam room feels like a real eye adjustment.
- Shadows: high-quality shadow maps; long directional shadows when the storm breaks.
- Volumetric fog + volumetric beam: `r.VolumetricFog` on, fog density driven by
  `BBWeatherController`.

## 2. The lighthouse beam (signature look)

The single most important visual object in the game.

- A physically-scaled spotlight: long attenuation (km-scale), narrow cone, warm colour.
- Visible as a *volume* through fog and rain: volumetric fog contribution plus fog-driven
  density so the beam "reads" as a solid searchlight from a distance.
- Distinct states: powered sweep (clean, steady), dying/failed (flicker, sagging
  intensity, colour droop), dead (nothing).
- A subtle "searching" behaviour in auto mode: the beam always looks motivated.

## 3. Environment targets

- **Rocks/wet surfaces**: PBR materials, strong normal/roughness variation, a wetness
  system (0.2) that darkens albedo and tightens specular in rain zones.
- **Ocean**: believable storm sea (UE5 water plugin / high-quality normal-driven surface),
  dark, violent; foam on the rocks.
- **Storm sky**: cloud coverage, precipitation layers, wind.
- **Fog**: exponential height fog, density budgeted so the beam stays readable — fog is
  a *design tool*, not a hiding mechanism.
- **Vegetation**: wind-reactive (speed-tree / simple wind components), sparse, coastal —
  gorse, bent grass.

## 4. Interior targets (lighthouse & annex)

- Old industrial machinery: brass/dark iron/aged enamel, believable scale, grease.
- Realistic interiors: the lamp room glazed with patterned glass; the keeper's rooms
  barely lit; the generator annex cold and damp.
- Moonlight/doorlight through cracked window panes vs. artificial light once the
  generator runs — a warm/cool transition the player *feels*.

## 5. Effects & tech notes (UE5)

- **Niagara**: rain (directional streaks + surface splashes), mist layers near the sea,
  beam-assisted dust/moisture motes inside the cone, generator exhaust plume. (0.2 —
  assets are editor-created; architecture already exposes the hooks.)
- **Volumetric fog**: enabled; density/winds driven by the weather controller.
- **Lumen**: default GI path; interior flashlight moments rely on it.
- **Wetness**: material-based first (wetness mask), full system later.
- **Audio-visual pairing**: the beam, rain, and generator each have signature presence.

## 6. Content budget discipline

- No placeholder cubes posing as props outside the explicit greybox bootstrap world.
- Real assets in later milestones: modular lighthouse set, island rocks kit, storm sky,
  rain Niagara, lantern lens materials, keeper's props. Each asset must earn its place.
- Greybox-only today; the world is designed so "swap mesh, keep gameplay actor" is true
  for every interactable (gameplay lives in components/actors, visuals are children).