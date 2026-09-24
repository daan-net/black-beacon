# BLACK BEACON — Codex / Coding-Agent Handoff

Read me last. Everything else (vision, design, architecture, current state) is in
the root docs; here is the operational picture and the exact next steps.

## 0. Current handoff — 2026-09-24

UE 5.8.2 is installed at `/home/a1/WORK/_TOOLS/UE_5.8.2` (`~/UnrealEngine`).
M0.1 is accepted. M0.2 and scoped M1 remain open. Do not restart the generator,
stairs, controls, save/load, beam or reveal implementation: these already work.
The latest authorized task is **Storm World Identity V0.1**; consult the current
storm section of `CURRENT_STATE.md` for validation evidence and remaining gates.

The storm adds native moving volumetric clouds, a displaced sea with crest foam,
finite coastal ground, wind-aligned rain/spray, lightning and delayed thunder,
and four ambient sound layers with interior muffling. Source and regeneration
instructions are in `Art/Source/StormWorld/README.md`. Gameplay remains authoritative.
There are now **53** plain-C++ checks and **four** Unreal automation tests.

Run `./Tools/review_storm.sh` for native-1080p Vulkan gameplay regressions and A–J
captures. Evidence goes to `Saved/Screenshots/LinuxEditor/BlackBeacon_Storm_*`,
`Saved/Automation/StormWorldV01`, and `Saved/StormReview/`. The latter includes an
actual runtime audio recording. The previous visual implementation is preserved;
pre-change recovery is `Saved/Checkpoints/Before_StormWorld_V01.tar.gz` at `ed707a5`.

Renderer note: `r.Vulkan.Bindless.PreferredExtension=1` selects the supported
`VK_EXT_descriptor_heap` backend. Two consecutive full rendered suites passed;
the default descriptor-buffer path intermittently lost the GPU device on NVIDIA
595.91.07. Keep this tested compatibility setting; see CURRENT_STATE.md for limits.

Next gate is user visual/audio review plus representative performance testing,
not new mechanics or a whole-island redesign. The coastline, interior and wreck
still contain blockout art; do not claim final production quality or M1 completion.

---

## 1. Ground rules (binding — AGENTS.md)

- Slice scope (0.1) is fixed. No new features without a CHANGELOG scope note.
- **Status words are strict:** IMPLEMENTED = compiled in intended context;
  TESTED = built + exercised with evidence; never claim either for unbuilt code.
- **Logics layer stays engine-free** (no UE types) or the standalone tests break.
- Tunables live in `Config/*.ini` as `config`-tagged `UPROPERTY`s, or C++ data
  structs. No magic numbers in gameplay code.
- Run `./Tools/validate.sh` before every commit; keep it green.
- Update CHANGELOG.md + CURRENT_STATE.md **in the same commit** as any change.
- No engine substitution: if UE is unavailable, stop at the furthest
  non-corrupting point and say so in CURRENT_STATE.md.

## 2. Recurring commands

```bash
# local validation (no engine needed): project-file sanity + logic unit tests
./Tools/validate.sh

# logic tests only
cmake -S Tests -B Tests/build -G Ninja && cmake --build Tests/build && ./Tests/build/bb_logic_tests   # expect: ALL PASS - 53/53

# linux in-engine build (once an engine root exists)
<UE_ROOT>/Engine/Build/BatchFiles/Linux/Build.sh BlackBeaconEditor Linux Development -project="$PWD/BlackBeacon.uproject" -waitmutex
# windows: Engine\Build\BatchFiles\Build.bat BlackBeaconEditor Win64 Development -project=... -WaitMutex
```

`BlackBeacon.uproject` has `"EngineAssociation": ""` on purpose: a fresh engine
associates on first open (or set it to the installed version, e.g. `5.8.2`).

## 3. Unreal Engine setup

UE 5.8.2 is installed at `/home/a1/WORK/_TOOLS/UE_5.8.2`; `~/UnrealEngine`
points to it. The Linux editor target, project startup, NVIDIA Vulkan runtime,
and rendered automation are verified.

## 4. Repo map (short version)

```
BlackBeacon.uproject          valid JSON, EngineAssociation blank, targets Win+Linux
Config/*.ini                  DefaultEngine|Game|Input|Editor — all tunables live here
Source/BlackBeacon/           single runtime module
  Public/BlackBeacon/
    Logics/                   engine-free C++: BBBeamMath, BBRevealStateMachine,
                              BBObjectiveGraph, BBWeatherState  ← unit-tested
    Core/                     game mode/state, character, controller, BBProceduralWorld
    Interaction/              IBBInteractableInterface, UBBInteractionComponent,
                              UBBPromptWidget (code-only Slate)
    Power/                    UBBPowerSystem (world subsystem), source/consumer
                              interfaces, UBBGeneratorComponent
    Lighthouse/               ABBLighthouseController, UBBLighthouseBeamComponent,
                              UBBBeamRevealComponent
    Weather/                  UBBWeatherController (fog states; exposes fog
                              multiplier for beam/reveal readability)
    Objectives/               UBBObjectiveSystem (game-instance subsystem),
                              UBBObjectiveTriggerComponent
    Save/                     UBBSaveGame + UBBSaveSubsystem (tested save/load)
  Private/BlackBeacon/        mirrors Public one-to-one
Tests/                        CMake + standalone harness → bb_logic_tests (53 checks)
Tools/validate.sh             local gate: JSON/ini sanity + logic tests
Content/BlackBeacon/          persistent M0.2 map + small authored material set
```

Design notes you'll need:
- **The scanline:** beam → reveal is *event-driven*: the beam publishes
  `FBBBeamQuery`; reveal components subscribe and evaluate it themselves. Never
  search the world per-frame; never let reveal logic touch light components.
- **Objective wiring:** trigger components (overlaps/interact/system events)
  complete objective IDs declared in `DefaultGame.ini`. IDs in code, the ini, and
  `BBProceduralWorld` must match byte-for-byte. The 9 IDs start `BB_OBJ_ARRIVE`
  and end `BB_OBJ_DISCOVER_ANOMALY` (GAME_DESIGN §1.2).
- **Power wiring:** generator `Start()` (interactable) → spin-up timer → power
  network grants the lighthouse controller → beam powered. Beam always ticks
  (even unpowered) so reveals fade when the lantern dies — don't "optimize" that
  away.
- **Config contract:** adding a `[/Script/BlackBeacon.X]` section requires class
  `X` under `Source/` and every key must be a `config`-tagged `UPROPERTY` on `X`.
  `validate.sh` checks sections only; **you** keep keys in sync (manual until an
  automated key check is added).

## 5. Known fixes applied during bootstrap (don't reintroduce)

- `BBProceduralWorld.h` forward-declared weather/lighthouse mismatches — fixed.
- Stale `BindInputs` declaration removed from `BBPlayerController.h`.
- Added missing includes (`CollisionQueryParams`, `Pawn`, `StringConv`, `Text`, `World`).
- `NominalWatts` config key dropped (renamed) — key set == `UPROPERTY(config)` set.
- All `config`-tagged classes declared `config = Game`.
- A reveal-machine unit test failed because the *test* left the beam unpowered;
  machine logic was correct. Keep `bPowered = true` in beam fixtures.

## 6. Verified M0.1 checklist

1. `./Tools/validate.sh` → green.
2. Install/associate engine (§3); generate project files.
3. Build `BlackBeaconEditor` (Linux `Development` or Windows `Win64
   Development`) — verify the existing working target.
4. Run and walk the greybox slice: movement, sprint, crouch.
5. Interact generator → 4 s spin-up → power granted (watch objective tick).
6. Climb to the lantern room, start the beam, sweep manually → anomaly reveals
   (transient first reveal; fade on beam departure) → objective chain finishes; HUD prompt live.
7. Update `CURRENT_STATE.md` **and** `CHANGELOG.md` in the same commits, moving
   rows from *written* to `IMPLEMENTED`/`TESTED` only with evidence.
8. Commit with scoped messages, e.g. `BlackBeacon: validate storm world V0.1`.

## 7. Definition of done (M0.1 → hand off again)

1. Change written against current architecture. 2. Logic tests updated + passing
   (`validate.sh`). 3. Status docs updated honestly. 4. Clean commits, `.gitignore`
   respected. 5. Blocker → documented with next step, not worked around.

## 8. Things deliberately out of scope

Multiplayer, services, monetization, enemies/combat, survival/inventory systems,
and expansion beyond the approved first-reveal slice. The persistent map, rain,
save/load and storm audio are already implemented; they are no longer future scope.
