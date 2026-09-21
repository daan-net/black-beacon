# BLACK BEACON — Current State

> Keep this file honest and current: update it in the same commit as any code
> change. Status words per AGENTS.md: `IMPLEMENTED` (built in intended context),
> `TESTED` (built + exercised with evidence), `PLACEHOLDER` (to be replaced),
> `PLANNED` (paper only).

**Last updated:** 2026-09-21 · **Milestone:** M0 — Foundation & Bootstrap
**Latest build:** local `./Tools/validate.sh` → **green**; `bb_logic_tests` → **ALL PASS (45/45)**
**Engine status:** Unreal Engine **not installed** — UE C++ layer compiled = false.
**Repo history:** initial commits `bce6bc6` → `1f2bcf5` (6 commits, clean log — see CHANGELOG).

---

## 1. Where we are

M0 (foundation, docs, scaffold, logic layer, all UE systems written) is done on
this machine except the one thing it can't do: **compiling/running the UE layer,
which requires an installed engine.** M0.1 (engine bring-up) is blocked on that
install — see §5 for the exact blocker and next steps. No work was substituted
with a different engine or faked to "compile"; nothing here claims to run in-engine.

## 2. What works now (validated locally)

| Item | Status | Evidence |
|---|---|---|
| 10-doc set + git repo on `main` | `IMPLEMENTED` | files present; clean history |
| `.uproject` (valid JSON), `Config/*.ini`, module/Target/Build files | `TESTED` | `validate.sh` step 1 green |
| Config ↔ class cross-check | `TESTED` | `validate.sh` step 1: all 7 ini sections resolve |
| Logics layer: `FBBBeamMath`, `FBBRevealStateMachine`, `FBBObjectiveGraph`, `FBBWeatherInterpolator` | `TESTED` | `bb_logic_tests` **45/45 pass** (cmake+ninja, runs on this machine) |
| `Tools/validate.sh` | `TESTED` | exits 0; re-run after every change |

Manual (non-automated) check that has passed: every key in `DefaultGame.ini`
matches a `config`-tagged `UPROPERTY` name on the owning class.

## 3. UE systems — written, **not built**

All of these exist as real C++ in `Source/BlackBeacon/` and are consistent
(module wiring, forward decls, includes, config keys all reviewed), but they have
**never been compiled by Unreal Build Tool**. Per AGENTS.md they are therefore
**not `IMPLEMENTED` and not `TESTED`**. Treat them as untrusted until compiled.

| System | Files | Written | Built |
|---|---|---|---|
| Interaction (interface, component, prompt) | `Interaction/` | ✅ | ⬜ |
| Power (system, source/consumer, generator) | `Power/` | ✅ | ⬜ |
| Lighthouse (controller, beam, reveal) | `Lighthouse/` | ✅ | ⬜ |
| Weather (controller over interpolator) | `Weather/` | ✅ | ⬜ |
| Objectives (system, trigger) | `Objectives/` | ✅ | ⬜ |
| Save foundation (save game, subsystem) | `Save/` | ✅ | ⬜ |
| Core (game mode/state, character, controller) | `Core/` | ✅ | ⬜ |
| Greybox world builder | `Core/BBProceduralWorld` | ✅ | ⬜ |
| Module glue (`BlackBeacon.Build.cs`, module impl) | root | ✅ | ⬜ |

Content: `Content/` intentionally holds **no `.uasset`s** — the greybox slice
builds everything from engine basic shapes at runtime.

## 4. Known issues / limitations

- **Root blocker:** no Unreal Engine (see §5). Nothing in §3 compiled, so real
  compile errors may still hide there; the first M0.1 pass must assume that.
- **Not yet automated in `validate.sh`:** the ini-key ⇄ `UPROPERTY(config)`
  name match is checked manually today. A future improvement: grep headers for
  `UPROPERTY(config)` names and diff against ini keys per section.
- **Static checks only:** `validate.sh` confirms ini *sections* resolve to Source
  classes, not that every ini *key* is valid — keep the manual key check until
  the automated one exists.
- **Objective chain** must stay in sync between `DefaultGame.ini` and the
  trigger wiring in `BBProceduralWorld`/content (IDs are stable keys
  `BB_OBJ_*`; see GAME_DESIGN §1.2).
- Fixed during bootstrap (see CHANGELOG): reveal-test bug, forward-declaration
  mismatches, a stale declaration, missing includes, config key rename
  (`NominalWatts` dropped), `config = Game` on all config classes.

## 5. Blockers

**Unreal Engine is not installed**, and on this Linux box every official install
path is credential-gated:

- Linux has **no native Epic Launcher** for UE5 architectures built here.
- The engine's own GitHub source access requires an **Epic-linked GitHub account**
  (Epic must grant access to `EpicGames/UnrealEngine`).
- Full, current install steps: `CODEX_HANDOFF.md` → "Unreal Engine setup".

Per AGENTS.md §6, work stopped at the furthest non-corrupting point: everything
that can be validated without the engine is validated; the UE layer is left
written-but-unbuilt rather than rewritten around/without Unreal.

## 6. What would count as "working" (M0.1 acceptance)

1. Engine 5.4+ installed; project associates; `BlackBeacon` module compiles with
   zero errors (`BlackBeaconEditor ... Linux/Development` on Linux, or VS on
   Windows).
2. `Play` boots the greybox slice (procedural world, storm fog on, rain
   weather state).
3. Walk/look/sprint/crouch work; interaction prompt shows on generator &
   lighthouse control.
4. Generator interact → 4 s spin-up → power network grants the lighthouse.
5. Lighthouse control interact → beam powers on; **manual** rotation sweeps and
   reveals the anomaly; auto/manual modes both function.
6. The anomaly reveals on beam contact (2.5 s delay) and persists; whole 9-step
   objective chain completes.
7. Each of those has **evidence** (in-editor run, screenshot/session log) before
   any row in §3 is moved to `IMPLEMENTED`/`TESTED`.

## 7. Immediate next task

M0.1 engine bring-up (blocked on install): install UE per `CODEX_HANDOFF`,
generate project files, compile, then run the acceptance list in §6, fixing
everything that fails and updating this file + CHANGELOG.md in the same commits.