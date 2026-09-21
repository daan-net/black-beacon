# BLACK BEACON — Agent Rules (AGENTS.md)

These rules bind every coding agent (Codex, Claude, humans) working in this repository.
Read fully before changing anything.

## 0. Ground truths

1. **Keep the slice small.** Vertical-slice 0.1 scope is fixed (GAME_DESIGN.md §1).
   Adding a new feature requires a scope note in CHANGELOG.md and approval by the
   milestone owner. Ten unfinished mechanics are failure; one fixed thing is success.
2. **Never pretend it works.** Use the status words exactly:
   - `IMPLEMENTED` — code exists AND has been compiled/built in the intended context.
   - `TESTED` — built and exercised (unit test or in-engine run) with evidence.
   - `PLACEHOLDER` — scaffolding exists to be replaced (greybox, stubs).
   - `PLANNED` — designed on paper only.
   Update CURRENT_STATE.md every time you touch code.
3. **C++ and text/config first.** Do not port domain logic to Blueprint. Blueprints may
   wrap pre-built C++ systems for content-specific glue, never the reverse.
4. **No new dependencies without a note.** No plugins unless they solve a real problem;
   no third-party libraries; no services, telemetry, accounts, backends, monetization.
5. **Human-quality code.** Small files, one responsibility per class, UE conventions
   (`AB*` actors, `UB*` components, `IBB*` interfaces, `F*` plain structs), descriptive
   names, no giant god classes, no deeply coupled logic.

## 1. Architecture rules

- **Logics layer (no UE).** Any non-trivial simulation/decision logic goes in
  `Public/BlackBeacon/Logics/` as plain C++ (no UE types). UE actors/components are thin
  adapters. New logic MUST have unit tests in `Tests/`.
- **Event-driven.** Prefer delegates, timers, and subsystems over `Tick()`. If you must
  tick, tick the fewest things with the longest period that stays correct. Never search
  the world every frame.
- **System seam for the beam.** All illumination queries flow through
  `FBBBeamQuery` (Logics/B BeamMath). Do not let reveal logic reach into light
  components directly.
- **New subsystems** register via `UGameInstanceSubsystem`/`UWorldSubsystem` when they
  are cross-actor; use components when they belong to one actor.

## 2. Config & data rules

- Tunables live in `Config/*.ini` as `config`-tagged `UPROPERTY`s, or in C++ data
  structs. Never scatter magic numbers through gameplay code.
- Objective chain and any storyline data live in text/config, not in Blueprint assets.
- When a tunable changes an existing user-visible behaviour, mention it in CHANGELOG.md.

## 3. Code style

- Follow the style of the files around you: 4-space indent, braces on new lines
  (Allman), `#pragma once`, `#include` your `.generated.h` last.
- Private members `LowerCamelCase`; properties `UpperCamelCase`; constants `ALL_CAPS`-
  style where meaningful. Use `const`, `TConstArrayView`, `FStringView` sensibly.
- No `using namespace` in headers. Keep includes minimal.
- Comments explain *why*, not *what*. English only.

## 4. Validation before finishing

Run, from repo root:

```bash
./Tools/validate.sh
```

This must pass before you commit code changes. It checks project-file sanity and builds
+runs the logic-layer unit tests. In-engine validation (once UE is installed):

```bash
# Linux: generate + build
<UE_ROOT>/Engine/Build/BatchFiles/Linux/Build.sh BlackBeaconEditor Linux Development -project=<abs>/BlackBeacon.uproject -waitmutex
```

If you cannot run in-engine validation, say so honestly in CURRENT_STATE.md instead of
marking things TESTED.

## 5. Repository hygiene

- Git history is clean and logical, one concern per commit. Message style:
  `BlackBeacon: begin power system — generator start flow` (scope + short summary).
- Never commit: `Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Saved/`, `Build/`,
  `.vscode/`, IDE files. Covered by `.gitignore` — keep it that way.
- Never commit engine or editor state, `.uasset` working-in-progress noise, or logs.
- Update `CHANGELOG.md` and `CURRENT_STATE.md` in the same commit as the change.

## 6. When you are blocked

- Engine unavailable → do NOT rewrite around/without Unreal. Document the exact blocker
  in CURRENT_STATE.md, finish everything that is valid without the engine, and stop at
  the furthest non-corrupting point.
- Credentials/paid services/licensing/product-direction conflict → stop and ask.

## 7. Definition of done (for any task)

1. Change written against the current architecture (not against a private fork of it).
2. Logic-layer unit tests updated + passing (`./Tools/validate.sh`).
3. CURRENT_STATE.md / CHANGELOG.md updated with honest status.
4. Clean commit(s) made, `.gitignore` respected.
5. If the task needs the engine and the engine is absent, done = task documented as
   blocked with a precise next step.