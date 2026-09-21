# BLACK BEACON — Milestones

Scope discipline: each milestone is small, complete, and shippable on its own.
QUALITY > FEATURE COUNT. Do not start the next milestone until the current one is
built, tested, and bumped in CURRENT_STATE.md.

## M0 — Foundation & Bootstrap (current)

Repository, documentation set, UE5 C++ project scaffold, plain-C++ logic layer with
unit tests, greybox bootstrap world, git history, handoff docs.

**Acceptance (M0)**
- [x] Repo + docs (all 10 top-level markdown files present and consistent)
- [x] Valid `.uproject`, `Config/*.ini`, UE module/Target/Build files
- [x] Logic layer compiles & its unit tests pass on this machine (`./Tools/validate.sh`)
- [x] ALL UE C++ systems written (interaction, power, beam, reveal, weather, objectives,
      save foundation, greybox builder) — unbuilt until engine drops in
- [x] Honest CURRENT_STATE.md + CODEX_HANDOFF.md
- [ ] Unreal Engine installed; project generates; UE systems compile — **blocked on engine**

## M0.1 — Engine bring-up (Phase A complete; Phase B in progress)

Install UE 5.4+ (see CODEX_HANDOFF § Unreal setup), generate VS/Linux project files,
compile the module, run on the greybox slice, fix compile/runtime errors.

**Acceptance (M0.1)**
- [x] `BlackBeacon` module compiles with zero errors (warnings avoided)
- [ ] Play the greybox slice: walk/look/sprint/crouch work
- [ ] Interact with generator → spin-up → power on
- [ ] Climb to lantern room, start the beam, manual rotation works
- [ ] Beam is visibly volumetric through fog
- [ ] First anomaly reveals and fades per config
- [ ] Whole objective chain completes; HUD prompt updates

## M0.2 — Vertical slice 0.1 polish (first "playable prototype")

Authored greybox level (first real `.umap`), weather storm state, rain Niagara,
prompt/HUD polish, save/load seam, audio placeholders, performance sanity check.

**Acceptance (M0.2)**
- [ ] Authored map replaces procedural builder as default (builder kept as dev tool)
- [ ] Storm + fog + rain active; beam readable in all weather states
- [ ] Save/load restores generator/beam/objective state
- [ ] Stable 60+ fps on target PC config at reasonable settings

## M0.3 — Visual/audio pass

Wet surfaces, storm sky, ocean, wind-reactive vegetation, polished volumetric beam,
signature beam + reveal sound design, filmic grade.

## M1+ — Design expansion (planned)

- [ ] More BeamReveal content (audio triggers, story/event triggers, persistent set)
- [ ] Lighthouse interior set (keeper's quarters, history in objects)
- [ ] Village corner + island bay
- [ ] Story layer: environmental storytelling entry points
- [ ] Beam filters/lenses (coloured lanterns, optics)
- [ ] Day-0 production build config

## Never in scope

Multiplayer, services, monetization, crafting/survival systems, enemy/combat systems,
procedural world generation as product, feature-count-driven content.
