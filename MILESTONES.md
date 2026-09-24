# BLACK BEACON — Milestones

Scope discipline: each milestone is small, complete, and shippable on its own.
QUALITY > FEATURE COUNT. Do not start the next milestone until the current one is
built, tested, and bumped in CURRENT_STATE.md.

## Storm World Identity V0.1 — TESTED, user review pending (2026-09-24)

Authorized environment pass within the existing first-reveal slice. Native moving
storm clouds/shadows, rough sea/foam, wind-driven rain and coastal spray/mist,
lightning/delayed thunder and four ambient layers are built and exercised.
53/53 logic checks and two consecutive 4/4 rendered Unreal suites pass at native
1080p with the tested descriptor-heap Vulkan compatibility setting.

- [x] Preserve and exercise generator / power / stairs / beam / reveal / save flow
- [x] Produce A–J actual renders plus a repeat cloud view and runtime audio recording
- [ ] User visual/audio review of movement, storm identity, mix and remaining art limitations
- [ ] Representative stable-60-FPS gameplay benchmark (capture sample: 50.26 FPS)

See CURRENT_STATE.md for exact evidence and known limits. This checkpoint does not
close M0.2 or M1. No expansion beyond the approved slice is authorized by this result.

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
- [x] First anomaly reveal behavior is configurable and exercised in-engine
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
- [ ] Stable 60+ fps on target PC config at reasonable settings (latest 1080p Vulkan automation warm-up: 43.43 FPS; gameplay benchmark still needed)

M0.2 remains open. The world-anchored instanced rain field is accepted after direct user review; the lighthouse interior stays dry. Stair look now clamps vertical camera movement to a moderate range, allowing the player to inspect steps while descending and keeping horizontal steering unrestricted; the revised Vulkan PlayerControls test passed, with direct feel review still pending. The latest 1080p Vulkan automation warm-up reached 43.43 FPS, below the 60 FPS gate and not a representative gameplay benchmark. Coast boulders now retain blocking collision; decorative wreck pieces are not a traversal surface. The milestone owner has authorized a scoped M1 first-reveal implementation while M0.2 remains open; do not expand into unrelated M1 work.

## M1 — The First Reveal (in progress, scoped)

The existing generator, power, stair, beam, weather, objective, save, and reveal systems
already cover the route to the first anomaly. Avoid rebuilding these systems. The first
reveal now evaluates tagged mesh parts independently through the existing beam query,
fades transient parts when the beam leaves, and activates a next objective on discovery.

**Current gate:** mechanics and automation are `TESTED`; the wreck now combines a
low-poly authored hull with individually revealed frame/mast blockout pieces. Paired
Vulkan renders show the hull revealed by the beam and hidden after it leaves. A
generated weathered paint albedo now improves tower readability in the first-person
shore view, but the close view crops its lantern. The surrounding coast, path,
lighthouse interior dressing, and reveal audio remain `PLACEHOLDER` or `PLANNED`, so
M1 remains open. Continue only within the approved 10–15 minute first-reveal slice.

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
