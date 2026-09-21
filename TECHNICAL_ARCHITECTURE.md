# BLACK BEACON — Technical Architecture

Target engine: **Unreal Engine 5 (5.4+ baseline)**. PC-first, C++-first, single-player.

## 1. Principles

1. **C++ and text/config first.** All game logic lives in C++; all tunables travel
   through `Config/*.ini` or data structures defined in C++. Blueprints only for things
   where they give a concrete Unreal advantage (per-asset materials, event graph glue in
   content, UI polish) — never for domain logic.
2. **Small, modular systems.** One class = one job. Prefer components/interfaces/subsystems
   over god actors. No deeply coupled gameplay logic.
3. **Event-driven over polling.** No per-frame global searches, no gratuitous `Tick()`.
   Periodic timers are acceptable where a tick every frame adds nothing.
4. **Testable logic.** Anything with meaningful decision/simulation logic is implemented
   as a plain C++ class in the `Logics` layer (no UE dependencies) so it can be unit-tested
   without the engine. UE actors/components are thin adapters over this logic.
5. **No external services.** No online, accounts, telemetry, backends, monetization,
   no unnecessary plugins/dependencies.
6. **Honest status.** IMPLEMENTED / TESTED / PLACEHOLDER / PLANNED are distinguished in
   `CURRENT_STATE.md`. Never claim something is built unless it was built and run.

## 2. Module layout

Single runtime module **`BlackBeacon`** (`Source/BlackBeacon/`) — appropriate for the
vertical-slice scale. If it grows, split into `BlackBeaconCore` / `BlackBeaconGameplay`
later; the folder structure below is already arranged so that split is mechanical.

```
Source/BlackBeacon/
├── BlackBeacon.Build.cs
├── Public/BlackBeacon/
│   ├── BlackBeacon.h                     # module include
│   ├── Logics/                           # plain C++ (no UE) — unit-tested
│   │   ├── BBBeamMath.h/.cpp
│   │   ├── BBRevealStateMachine.h/.cpp
│   │   ├── BBObjectiveGraph.h/.cpp
│   │   └── BBWeatherState.h/.cpp
│   ├── Core/                             # framework
│   │   ├── BBGameMode.h/.cpp
│   │   ├── BBPlayerController.h/.cpp
│   │   ├── BBPlayerCharacter.h/.cpp
│   │   ├── BBGameState.h/.cpp
│   │   └── BBProceduralWorld.h/.cpp      # greybox bootstrap builder
│   ├── Interaction/
│   │   ├── BBInteractableInterface.h     # IBBInteractableInterface
│   │   ├── BBInteractionComponent.h/.cpp
│   │   └── BBPromptWidget.h/.cpp         # Slate prompt (code-only UI)
│   ├── Power/
│   │   ├── BBPowerSystem.h/.cpp          # world subsystem
│   │   ├── BBPowerSourceInterface.h      # IBBPowerSourceInterface
│   │   ├── BBPowerConsumerInterface.h    # IBBPowerConsumerInterface
│   │   └── BBGeneratorComponent.h/.cpp
│   ├── Lighthouse/
│   │   ├── BBLighthouseController.h/.cpp # AActor: machinery + lens home
│   │   ├── BBLighthouseBeamComponent.h/.cpp  # signature system
│   │   └── BBBeamRevealComponent.h/.cpp  # reveal/ anomaly component
│   ├── Weather/
│   │   └── BBWeatherController.h/.cpp
│   ├── Objectives/
│   │   ├── BBObjectiveSystem.h/.cpp      # game-instance subsystem
│   │   └── BBObjectiveTriggerComponent.h/.cpp
│   └── Save/
│       ├── BBSaveGame.h/.cpp             # USaveGame foundation
│       └── BBSaveSubsystem.h/.cpp        # minimal save/load seam
└── Private/BlackBeacon/                  # mirrors Public/ one-to-one
```

Naming: actors `AB*`, components `UB*`, plain classes `F*`/`T*` style, interfaces
`IBB*`. All types prefixed `BB` to avoid collisions. File names match class names.

## 3. System catalogue

### 3.1 Framework
- **`ABlackBeaconGameMode`** (AGameModeBase): defaults (pawn/controller/gamestate),
  invokes the procedural greybox bootstrap when configured, fallback player spawn.
- **`ABlackBeaconPlayerController`** (APlayerController): Enhanced Input mapping
  context created in code (no asset dependencies), input bindings, prompt widget.
- **`ABlackBeaconPlayerCharacter`** (ACharacter): movement tuning, sprint/crouch,
  interact component, spring-arm camera.
- **`ABlackBeaconGameState`** (AGameStateBase): UI-facing snapshot of objective/power/
  beam state, updated by subscribing to subsystems.

### 3.2 Interaction
- **`IBBInteractableInterface`** — `GetInteractionPrompt()`, `OnInteract()`.
- **`UBBInteractionComponent`** — periodic (10 Hz timer) forward trace, focuses the
  nearest interactable, drives the prompt. No per-frame trace.
- **`UBBPromptWidget`** — code-only `UUserWidget` (Slate text) showing "Press E …".

### 3.3 Power
- **`UBBPowerSystem : UWorldSubsystem`** — registry of sources/consumers; event-driven
  recalculation; `FBBPowerNetworkChanged` multicast.
- **`IBBPowerSourceInterface`** — `GetCurrentWatts()`, `IsSourceActive()`.
- **`IBBPowerConsumerInterface`** — `GetDemandWatts()`, `OnPowerGranted/Revoked` events.
- **`UBBGeneratorComponent`** — start/stop, spin-up timer, interface implementation,
  interactable hook.

### 3.4 Lighthouse & beam
- **`ABBLighthouseController`** — owns the beam component, is the power consumer,
  later the lens/machinery representation. The "START LIGHTHOUSE" interactable.
- **`UBBLighthouseBeamComponent (USceneComponent)`** — the signature system.
  `ERotationMode { Off, Auto, Manual }`; configurable speeds; target/current intensity
  lerp; flicker profiles; publishes `FBBBeamQuery`. Drives a spawned `USpotLightComponent`
  for the visible + volumetric contribution. `SubscribeRevealComponent()`.
- **`UBBBeamRevealComponent`** — wraps `FBBRevealStateMachine`; consumes beam queries;
  toggles owner visibility (+ optional material fade param). Auto-subscribes to the
  world's beam component when configured.

### 3.5 Weather
- **`ABlockbbWeatherController`**/`UBBWeatherController` — owns `UExponentialHeightFogComponent`,
  drives `FBBWeatherInterpolator` over states Clear/Fog/Rain/Storm, exposes fog-density
  multiplier for the beam/reveal systems.

### 3.6 Objectives
- **`UBBObjectiveSystem : UGameInstanceSubsystem`** — resolves the objective graph
  (`FBBObjectiveGraph`), chain defined in `Config/DefaultGame.ini` (text-driven),
  emits `OnObjectiveActivated/Completed`.
- **`UBBObjectiveTriggerComponent`** — generic bridge: overlap-volume, interact, or
  event trigger → `CompleteObjective(Id)`.

### 3.7 Save
- **`UBBSaveGame : USaveGame`** + **`UBBSaveSubsystem`** — foundation only in 0.1:
  world-state structs (generator state, power, persistent reveals, objective progress).
  Serialization wired in 0.2.

## 4. Data flow (key paths)

```
interact (E) ──> IBBInteractableInterface::OnInteract
                  └─> UBBGeneratorComponent::Start()
                        └─> UBBPowerSystem (source active)
                              └─> recalculate → consumer (lighthouse) granted
                                    └─> UBBLighthouseController → beam powered
                                          └─> beam updates FBBBeamQuery each state change
                                                └─> UBBBeamRevealComponent::Evaluate(query)
                                                      └─> reveal state machine → visibility
Objective triggers (overlap/interact/events) ──> UBBObjectiveSystem::CompleteObjective
Weather controller ── fog density ──> beam/reveal readability (multiplier)
```

## 5. Config-driven values (0.1)

- `DefaultEngine.ini` — renderer/system settings (volumetric fog on, etc.), game mode,
  default map.
- `DefaultGame.ini` — `[/Script/BlackBeacon.BBObjectiveSystem]` objective chain;
  `[/Script/BlackBeacon.BBGameMode]` bootstrap world + fallback spawn transform;
  subsystem tunables (flicker profile, weather palette, generator spin-up, beam speeds).
- `DefaultInput.ini` — minimal mouse capture settings (Enhanced Input live in code).

All tunables are read via `config`-tagged `UPROPERTY`s on the owning class so they are
visible, settable in the editor, and auditable as text.

## 6. Bootstrapping without an editor

Until art is in, `ABlackBeaconGameMode` can build a **procedural greybox slice** at
runtime from engine basic shapes (cylinder tower, box annex, plane ground/ocean, trigger
volumes, player start). This keeps the entire project text-driven, runnable on a fresh
engine install with no `.uasset` dependencies. Real authored maps replace it wholesale
in the milestone-0.1 editor pass; the builder stays as a dev-only tool.