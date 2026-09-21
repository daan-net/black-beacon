# BLACK BEACON — Read Me First

A cinematic first-person 3D mystery/exploration game set on a remote, storm-battered
North Atlantic island dominated by an old lighthouse. You arrive to investigate why
the automated lighthouse stopped transmitting. Its beam is the signature mechanic:
it reveals things that are invisible under normal lighting.

**Engine:** Unreal Engine 5 (C++‑first) · **Platform:** PC · **Genre:** mystery/exploration
**Mood:** mysterious, lonely, cinematic, atmospheric, physically believable, beautiful.

---

## Before anything else

1. **Unreal Engine is not installed on this machine yet** — see
   [CODEX_HANDOFF.md](CODEX_HANDOFF.md) § "Unreal Engine setup" for exact install steps.
   The repository is a complete, real UE5 C++ project; drop the engine in and it builds.
   Nothing here substitutes a different engine, and no engine were used to fake it.
2. Read [MASTER_VISION.md](MASTER_VISION.md) for the one-paragraph vision.
3. Read [CURRENT_STATE.md](CURRENT_STATE.md) for the honest, always-updated status map
   (what works, what is tested, what is placeholder, next task).
4. Read [AGENTS.md](AGENTS.md) if you are a coding agent working in this repo.
5. If you are the agent continuing development, read [CODEX_HANDOFF.md](CODEX_HANDOFF.md) last.

## Repository map

```
BLACK_BEACON/
├── BlackBeacon.uproject          # UE project file (valid JSON, engine-agnostic)
├── Config/                       # Default*.ini — engine/game/input configuration (text, auditable)
├── Source/BlackBeacon/           # C++ runtime module "BlackBeacon"
│   ├── Public/BlackBeacon/       #   headers (systems, components, plain-C++ logic)
│   └── Private/BlackBeacon/      #   implementations
├── Tests/                        # Standalone logic-layer unit tests (builds WITHOUT UE)
├── Tools/validate.sh             # Local validation: JSON/ini checks + logic tests
├── Content/                      # Unreal content folders (no .uassets yet — editor-created later)
└── *.md                          # This documentation set (see below)
```

## Documentation set

| File | Purpose |
|---|---|
| [MASTER_VISION.md](MASTER_VISION.md) | Product vision, pillars, target feeling |
| [GAME_DESIGN.md](GAME_DESIGN.md) | Mechanics, beam design, objective chain, scope discipline |
| [TECHNICAL_ARCHITECTURE.md](TECHNICAL_ARCHITECTURE.md) | System architecture, module layout, conventions |
| [VISUAL_DIRECTION.md](VISUAL_DIRECTION.md) | Visual / audio / atmosphere targets and UE5 techniques |
| [MILESTONES.md](MILESTONES.md) | Milestone plan, current milestone, acceptance criteria |
| [CHANGELOG.md](CHANGELOG.md) | Dated change log |
| [CURRENT_STATE.md](CURRENT_STATE.md) | **Always-current** status: milestone, works/not-works, bugs, next task |
| [CODEX_HANDOFF.md](CODEX_HANDOFF.md) | Technical handoff for the next coding agent |
| [AGENTS.md](AGENTS.md) | Development rules for all future coding agents |

## Building today

There is nothing to build yet: **the Unreal Engine toolchain is missing**.

What *can* be validated right now, without UE:

```bash
./Tools/validate.sh     # JSON/ini sanity + compiles & runs the logic-layer unit tests
```

The logic layer (`Source/BlackBeacon/Public/BlackBeacon/Logics/`) is plain, engine-free C++
(beam math, beam-reveal state machine, objective graph resolver, weather interpolation)
that is unit-tested by `Tests/`. Those tests run on any machine with a C++ compiler.

The UE C++ systems (components/actors/subsystems) are **written but not compiled** until
Unreal Engine is installed. Nothing that has not actually been built is ever claimed as
working — see [CURRENT_STATE.md](CURRENT_STATE.md) for the exact status of each system.

## Ground rules (abridged)

- QUALITY > FEATURE COUNT. No placeholder-cube props pretending to be final assets.
- C++ and text/config first. No opaque Blueprint spaghetti. Blueprints only where they
  give a clear Unreal-specific advantage (e.g. per-asset material tweaks, UI polish).
- No online services, accounts, multiplayer, telemetry, backend, monetization.
- Vertical slice 0.1 scope is fixed — see [GAME_DESIGN.md](GAME_DESIGN.md).
- If you change anything, update [CHANGELOG.md](CHANGELOG.md) and [CURRENT_STATE.md](CURRENT_STATE.md).