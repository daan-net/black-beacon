# BLACK BEACON — Change Log

Format: `date — milestone — summary`. Version reflects the vertical slice
(`0.x`); no released build exists yet. A change that alters an existing
user-visible behaviour is called out as **user-visible**.

Status vocabulary used here (same as AGENTS.md): `IMPLEMENTED` (built in the
intended context), `TESTED` (built + exercised with evidence), `PLACEHOLDER`
(scaffolding to be replaced), `PLANNED` (paper only). Anything written-but-not-
compiled is marked **written, not built** and is *not* `IMPLEMENTED`.

---

## 2026-09-21 — M0 — Initial foundation bootstrap

The repository as first established on `main`. No prior history. Unreal Engine
is **not installed** on this machine, so the UE C++ layer is written but not
compiled; every UE system below is **written, not built** until M0.1 pass.

### Added

- **Documentation set (10 root docs):** `README_FIRST`, `AGENTS`, `MASTER_VISION`,
  `GAME_DESIGN`, `TECHNICAL_ARCHITECTURE`, `VISUAL_DIRECTION`, `MILESTONES`,
  `CHANGELOG`, `CURRENT_STATE`, `CODEX_HANDOFF`. Scope anchored to vertical
  slice 0.1 (GAME_DESIGN §1).
- **UE5 C++ project scaffold:** `BlackBeacon.uproject` (valid JSON,
  `EngineAssociation` blank for drop-in engine choice), `Config/DefaultEngine|Game|
  Input|Editor.ini`, `Source/BlackBeacon.Target.cs`, `Source/BlackBeaconEditor.Target.cs`
  (`.Latest` engine versions), `Source/BlackBeacon/BlackBeacon.Build.cs` (module
  `BlackBeacon`, runtime, minimal dependency surface). `.gitignore` covers all
  Unreal build artifacts.
- **Logics layer (plain C++, no UE)** under `Public/BlackBeacon/Logics/`:
  `FBBBeamMath` (beam/cone query), `FBBRevealStateMachine` (reveal timing),
  `FBBObjectiveGraph` (prerequisite-aware chain resolver), `FBBWeatherInterpolator`
  (Clear/Fog/Rain/Storm). `TESTED` — 45/45 checks pass via `Tests/` (CMake +
  standalone harness, no engine, no external framework).
- **Interaction:** `IBBInteractableInterface`, `UBBInteractionComponent`
  (10 Hz forward trace, prompt focus), `UBBPromptWidget` (code-only Slate prompt).
- **Power:** `UBBPowerSystem` (world subsystem, event-driven source/consumer
  registry), `IBBPowerSourceInterface`, `IBBPowerConsumerInterface`,
  `UBBGeneratorComponent` (spin-up timer + interactable).
- **Lighthouse:** `ABBLighthouseController` (power consumer + beam control
  interactable), `UBBLighthouseBeamComponent` (rotation Off/Auto/Manual,
  intensity lerp, flicker, power state, publishes `FBBBeamQuery`,
  subscribes reveal components — the signature reusable system),
  `UBBBeamRevealComponent` (data-driven reveal over the state machine).
- **Weather:** `UBBWeatherController` (fog + weather states, exposes fog-density
  multiplier for beam/reveal readability).
- **Objectives:** `UBBObjectiveSystem` (game-instance subsystem over
  `FBBObjectiveGraph`, chain text-driven from `DefaultGame.ini`),
  `UBBObjectiveTriggerComponent` (generic volume/interact/event trigger).
- **Save foundation:** `UBBSaveGame` + `UBBSaveSubsystem` (0.1: world-state
  structs only; serialization planned for 0.2).
- **Core:** `ABlackBeaconGameMode` (defaults + bootstrap flag + fallback
  player start), `ABlackBeaconPlayerController` (code-created Enhanced Input,
  manual beam aim), `ABlackBeaconPlayerCharacter` (sprint/crouch + interaction),
  `ABlackBeaconGameState`, `UBBProceduralWorld` (greybox slice builder wiring
  the whole loop — runnable with zero authored assets).
- **Config:** `DefaultGame.ini` carries all tunables and the 9-step objective
  chain `BB_OBJ_ARRIVE` → `BB_OBJ_DISCOVER_ANOMALY`. Every ini section maps to
  a `config`-tagged `UPROPERTY` class (cross-check in `validate.sh`).
- **Tooling:** `Tools/validate.sh` — project-file sanity (`.uproject` JSON,
  ini section → class cross-check) + builds/runs the logic tests. `PASSES`
  green on this machine.

### Fixed during bootstrap (pre-commit review)

- Forward-declaration mismatches in `BBProceduralWorld.h` (weather/lighthouse).
- Removed a stale `BindInputs` declaration in `BBPlayerController.h`.
- Missing `CollisionQueryParams`/`Pawn`/`StringConv`/`Text`/`World` includes.
- Config key removed/renamed (`NominalWatts` dropped) so keys match
  `UPROPERTY(config)` names exactly.
- All `config`-tagged classes set to `config = Game` in their headers.
- Test-side bug in the reveal machine test (beam left unpowered) fixed; the
  machine logic itself was correct.

### Notes

- **No new dependencies.** No plugins, no third-party libraries, no services,
  telemetry, accounts, backends, or monetization.
- Beams/flicker/weather palette are `PLANNED` for depth beyond 0.1 defaults;
  audio and authored content are out of 0.1 scope.
- Full UE build/run status is tracked in `CURRENT_STATE.md`; M0.1 acceptance
  criteria live in `MILESTONES.md`.