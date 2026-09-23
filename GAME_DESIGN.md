# BLACK BEACON — Game Design

## 1. The vertical slice (0.1)

A small coastal section of the island, one lighthouse (exterior + interior), and a
complete, convincing gameplay loop built around the beam. Exactly one example of the
signature Beam-Reveal mechanic, built on a reusable system.

The slice opens as the player regains consciousness on the wreck coast. Moonlight and
the silhouette of the unpowered lighthouse establish the destination without a
cutscene or waypoint. The asylum and illegal-experiment mystery belongs to the wider
story; 0.1 may imply it through the single reveal but does not add those locations.

### 1.1 Concrete player verbs (0.1)

| Verb | Implementation |
|---|---|
| walk / look | character + camera (see player section) |
| sprint | hold-to-sprint, FOV kick, stamina not in 0.1 |
| crouch | toggle crouch (capsule/camera lower, slower walk) |
| interact | generic interaction component + interface (press E) |

### 1.2 The 0.1 objective chain

```
ARRIVE                 → arrive at the landing point (completed on spawn)
→ ENTER LIGHTHOUSE     → enter the lighthouse door volume
→ FIND GENERATOR       → reach generator in the engine annex
→ START GENERATOR      → interact with generator (spin-up, power flows)
→ RESTORE LIGHTHOUSE POWER → automatic (power network asserts after generator is up)
→ CLIMB TO LANTERN ROOM    → reach the lantern floor volume
→ START LIGHTHOUSE     → interact with the beam/machinery control
→ AIM / ROTATE BEAM    → player takes manual control of the beam rotation
→ DISCOVER FIRST ANOMALY   → beam-reveal object becomes visible (persistent reveal)
```

Objectives are data (ID + text) resolved by `BBObjectiveSystem`; completion is triggered
by generic trigger components (overlap volumes, interact events, system events), never by
hard-coded sequencing in the player controller.

## 2. The signature mechanic: the Lighthouse Beam

Design goals:

- **Physical rotation** — yaw rotation driven by a rotation mode state machine.
- **Configurable rotation speed** — auto-rotation (deg/sec) and manual control rate.
- **Player/manual control** — the player can aim the beam (Yaw + Pitch within limits).
- **Automatic rotation** — the classic sweeping patrol mode.
- **Beam intensity** — target + current (lerped), with flicker and failure states.
- **Beam range / width** — attenuation radius + cone half-angle.
- **Volumetric appearance** — the component exposes what the visuals need
  (intensity, color, cone, fog boost) so any renderer (UE volumetric fog, Niagara,
  light shafts) can bind to it. 0.1 uses a Spotlight driven by the component.
- **Power state** — the beam is a power consumer; no generator → dead lantern.
- **Flickering/failure states** — data-driven flicker profile; dead → no light + hum cut.
- **Interaction with BeamReveal actors** — the beam publishes a beam query; reveal
  components respond. Reusable, not scripted.

### 2.1 Beam query contract

Each update the beam computes:

```
FBBBeamQuery {
  FVector  Origin;            // world-space beam origin (lamp)
  FVector  Direction;         // normalized, world-space
  float    HalfAngleRad;      // cone half-angle
  float    Intensity01;       // 0..1 current intensity (after lerp/flicker/power)
  bool     bPowered;          // hard power truth
  float    Range;             // cone length (cm)
}
```

Any reveal object can be illuminated at any time by evaluating the query — this is the
reusable seam between "the beam" and "everything the beam reveals".

## 3. Beam Reveal (anomaly) design

`BBBeamRevealComponent` (on any actor) implements a configurable, data-driven reveal:

| Parameter | Meaning |
|---|---|
| `RevealDelay` | seconds the beam must rest on the object before it starts revealing |
| `FadeTime` | seconds of fade in/out |
| `MinBeamIntensity` | minimum beam intensity to count as "illuminated" |
| `VisibilityDuration` | after full reveal, seconds it stays fully visible (≤0 = while illuminated) |
| `bPersistentReveal` | once revealed, stays visible forever |
| `bTriggerObjective` | objective ID to complete on first full reveal |
| `AudioTrigger` | optional (reserved for 0.2) |

Behaviour in 0.1: invisible normally → beam rests on it → gradually appears →
beam leaves → it fades or stays (per persistence). One anomaly in the slice: a structure
that has been there all along. Reusability is the requirement, not more content.

## 4. Power design

A small, honest grid:

- **`BBPowerSystem`** (world subsystem): registry of sources and consumers. Event-driven;
  recalculates the network only when something registers or toggles.
- **`IBBPowerSourceInterface`** — implemented by `BBGeneratorComponent` (active, watts).
- **`IBBPowerConsumerInterface`** — implemented by the lighthouse; requests power.
- Generator spin-up: start → warms up (progressive wattage over seconds) → power flows.
  This couples the "RESTORE POWER" beat to a small physical delay for believability.

## 5. Weather design

`BBWeatherController` (actor) drives a state machine: Clear → Fog → Rain → Storm,
with timed interpolations. It owns an `ExponentialHeightFogComponent` and drives
fog density/color, wind strength, and rain intensity (Niagara rain binds in 0.2;
storm-level fog is the 0.1 win). The beam's visibility *through* fog is a stated
visual goal, so the weather controller exposes a fog-density multiplier the beam
and reveal systems can read.

## 6. Interaction design

- `BBInteractionComponent` on the player: periodic short forward trace (not every frame),
  remembers the focused `BBInteractableInterface` actor, shows a prompt.
- `BBInteractableInterface`: `GetInteractionPrompt()` + `OnInteract(controller)`.
- Interactables in 0.1: generator, beam/machinery control.

## 7. Scope discipline (do not add)

- No inventory, crafting, survival stats, currencies.
- No enemies, no combat, no health system.
- No branching dialogue. One interactable "story beat" mechanism at most.
- No huge world. One bay, one village corner, one lighthouse.
- No full story. The island's past is present in the environment only.

## 8. Audio direction (0.2+, noted now)

Wind layers, rain on surfaces, distant ocean, mechanical groan and spin of the beam,
generator rumble, a power hum that *cuts* when power fails. The beam gets a signature
sound when it is "searching". The reveal has a subtle, non-cheap audio cue.
