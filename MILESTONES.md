# BLACK BEACON — Milestones

Scope discipline: each milestone is small, complete, and shippable on its own.
QUALITY > FEATURE COUNT. Do not start the next milestone until the current one is
built, tested, and bumped in CURRENT_STATE.md.

## M0 — Foundation & Bootstrap (complete)

Repository, documentation set, UE5 C++ project scaffold, plain-C++ logic layer with
unit tests, greybox bootstrap world, git history, handoff docs.

**Acceptance (M0)**
- [x] Repo + docs (all 10 top-level markdown files present and consistent)
- [x] Valid `.uproject`, `Config/*.ini`, UE module/Target/Build files
- [x] Logic layer compiles & its unit tests pass on this machine (`./Tools/validate.sh`)
- [x] ALL UE C++ systems written and built (interaction, power, beam, reveal, weather,
      objectives, save foundation, greybox builder)
- [x] Honest CURRENT_STATE.md + CODEX_HANDOFF.md
- [x] Unreal Engine 5.8.2 installed; UE systems compile

## M0.1 — Engine bring-up (complete)

Build the UE 5.8.2 module, run the greybox slice, and verify the complete player path.

**Acceptance (M0.1)**
- [x] `BlackBeacon` module compiles with zero errors (warnings avoided)
- [x] Play the greybox slice: walk/look/sprint/crouch work (in-engine input automation)
- [x] Interact with generator → spin-up → power on (in-engine flow automation)
- [x] Climb to lantern room, start the beam, manual rotation works (stair traversal and gameplay-flow automation)
- [x] Beam is visibly volumetric through fog
- [x] First anomaly reveals and persists per current config (in-engine flow automation)
- [x] Whole objective chain completes; HUD prompt updates (in-engine flow automation)

Rendered captures show the beam on/off, impact, and anomaly reveal. Direct play review
accepted its moving fog appearance and the corrected stair entrance and transitions.
The flow automation reaches the lantern volume by teleporting the pawn; the separate
stair traversal test moves the character across all 84 steps.

## M0.2 — Vertical slice 0.1 polish (in progress)

Authored greybox level (first real `.umap`), weather storm state, rain presentation,
prompt/HUD polish, save/load seam, audio placeholders, performance sanity check.

**Acceptance (M0.2)**
- [x] Authored map replaces procedural builder as default (builder kept as dev tool)
- [x] Storm rain reads as rainfall from normal gameplay viewpoints; user confirms the lighthouse interior stays dry
- [x] Save/load restores generator, lighthouse/beam, objective, reveal, weather, and player state (GameplayFlow automation)
- [ ] Stable 60+ fps on target PC config at reasonable settings (latest 1080p Vulkan automation warm-up: 43.37 FPS; gameplay benchmark still needed)

M0.2 remains open. The world-anchored instanced rain field is accepted after direct user review; the lighthouse interior stays dry. Stair look now locks vertical camera movement on stair treads while leaving horizontal steering unrestricted; the Vulkan PlayerControls test passed, pending direct feel review. The latest 1080p Vulkan automation warm-up reached 45.69 FPS, below the 60 FPS gate and not a representative gameplay benchmark. Large coast rocks and wreckage still lack collision. Do not begin M1 until performance and remaining traversal gates are reviewed.

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
