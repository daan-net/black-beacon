# BLACK BEACON — Codex / Coding-Agent Handoff

Read me last. Everything else (vision, design, architecture, current state) is in
the root docs; here is the operational picture and the exact next steps.

> 2026-09-21 update: the engine-install blocker described below is resolved.
> UE 5.8.2 is installed at `~/WORK/_TOOLS/UE_5.8.2`; the editor target builds
> and the project opens. See CURRENT_STATE.md for live M0.1 Phase B status.

## 0. TL;DR

BLACK BEACON is a UE5 PC-first, C++-first, single-player mystery built around one
reusable mechanic: the lighthouse beam reveals things invisible under normal
light. M0 (foundation) is done locally **except** compiling the UE layer, which
requires an installed Unreal Engine (currently absent, see §3). A plain-C++
**Logics** layer is written and unit-tested (45/45). All UE systems are written,
consistent, and **unbuilt**.

Your job when you pick this up: get an engine, compile, run M0.1 acceptance
(`MILESTONES.md`), and fix what breaks — without expanding scope (GAME_DESIGN §1,
§7).

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

## 3. Unreal Engine setup (the blocker)

No engine is installed on this machine. Paths (as of 2026-09-21):

- **Windows (easiest, official):** install *Epic Games Launcher* → *Unreal
  Engine* → 5.4.x or newer (5.4+ is the documented baseline). Launcher-based
  installs don't need Epic GitHub access. Then open `BlackBeacon.uproject`.
- **Linux:** there is **no native launcher** for UE5 consumer builds on this
  platform. Options:
  1. Build the engine from source: requires an **Epic-linked GitHub account**
     (`EpicGames/UnrealEngine` access is granted per-account by Epic). Then use
     the `Linux/Build.sh` command above.
  2. Develop in the editor on a Windows machine/instance and keep this repo in
     sync; both Target files build Win64 out of the box.
- Do **not** pursue cracked/unofficial distributions; credentials/policy is a
  hard stop per AGENTS.md §6.

"Engine is working" = the `BlackBeacon` module compiles with zero errors **and**
`Play` boots the greybox slice (see §6 checklist).

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
Content/                      no .uassets — greybox is code-built via BBProceduralWorld
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

## 6. First session with an engine: M0.1 checklist

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
