# BLACK BEACON — Codex / Coding-Agent Handoff

Read me last. Everything else (vision, design, architecture, current state) is in
the root docs; here is the operational picture and the exact next steps.

> 2026-09-21 update: the engine-install blocker described below is resolved.
> UE 5.8.2 is installed at `~/WORK/_TOOLS/UE_5.8.2`; the editor target builds
> and the project opens. M0.1 is complete; see CURRENT_STATE.md for live status.

## 0. TL;DR

BLACK BEACON is a UE5 PC-first, C++-first, single-player mystery built around one
reusable mechanic: the lighthouse beam reveals things invisible under normal
light. M0 is complete. UE 5.8.2 is installed; the editor target builds and the
actual project opens. The plain-C++ Logics layer passes 45/45 tests, and M0.1
input, gameplay-flow, and stair traversal automation pass in the engine. Direct
play review accepted the moving beam and corrected stair route.

Continue with the scoped M0.2 work in `MILESTONES.md` without expanding the
vertical slice (GAME_DESIGN §1, §7).

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
cmake -S Tests -B Tests/build -G Ninja && cmake --build Tests/build && ./Tests/build/bb_logic_tests   # expect: ALL PASS - 45/45

# linux in-engine build (once an engine root exists)
<UE_ROOT>/Engine/Build/BatchFiles/Linux/Build.sh BlackBeaconEditor Linux Development -project="$PWD/BlackBeacon.uproject" -waitmutex
# windows: Engine\Build\BatchFiles\Build.bat BlackBeaconEditor Win64 Development -project=... -WaitMutex
```

`BlackBeacon.uproject` has `"EngineAssociation": ""` on purpose: a fresh engine
associates on first open (or set it to the installed version, e.g. `5.4`).

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
    Save/                     UBBSaveGame + UBBSaveSubsystem (foundation only)
  Private/BlackBeacon/        mirrors Public one-to-one
Tests/                        CMake + standalone harness → bb_logic_tests (45 checks)
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
   Development`) — fix compile errors; none are known, expect some anyway.
4. Run and walk the greybox slice: movement, sprint, crouch.
5. Interact generator → 4 s spin-up → power granted (watch objective tick).
6. Climb to the lantern room, start the beam, sweep manually → anomaly reveals
   (2.5 s delay, persistent) → objective chain finishes; HUD prompt live.
7. Update `CURRENT_STATE.md` **and** `CHANGELOG.md` in the same commits, moving
   rows from *written* to `IMPLEMENTED`/`TESTED` only with evidence.
8. Commit with scoped messages, e.g. `BlackBeacon: compile module against engine 5.4`.

## 7. Definition of done (M0.1 → hand off again)

1. Change written against current architecture. 2. Logic tests updated + passing
   (`validate.sh`). 3. Status docs updated honestly. 4. Clean commits, `.gitignore`
   respected. 5. Blocker → documented with next step, not worked around.

## 8. Things deliberately out of scope

Authored `.umap` content (M0.2+, the greybox builder stays as dev tool), rain
Niagara, audio, save serialization (foundation structs only), multiplayer,
services, monetization, enemy/combat, survival/inventory systems.
