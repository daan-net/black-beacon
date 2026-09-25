# V0.4 validation handoff

The previous pause instructions are superseded. The owner recovered and manually
reviewed checkpoint `1c0cf71` on `wip/v04-visual-rebuild-20260925`, tagged
`checkpoint-v04-playable-20260925`. Its Vulkan asset build completed successfully
at 22:26 UTC on 2026-09-24. Do not repeat that build or regenerate all assets.

## Work performed on resume

Read-only asset/reference audit and the remaining integrated validation were run.
The first suite found a real step-74 obstruction from the supplemental gallery
deck. Only the visual gallery and collision deck were regenerated/reimported,
with the hatch widened 112→200 degrees. Original stair collision, objectives,
beam/reveal logic and materials were preserved. Tests now cover the recorded
capsule/head obstruction as well as the full physical climb.

See CURRENT_STATE.md and V04_VALIDATION.md for the final run and evidence.
The original checkpoint/tag remains unchanged. Do not start V0.5 automatically.

## Reproducibility caveat

The committed OBJ and Unreal assets are the checkpoint's authoritative art.
The generator still contains pre-pause source-only changes to lantern cap
triangulation, stair lamp fixtures and annex roof winding that were not part of
the protected mesh export. Counts differ for lantern/stair source; recorded in
`Saved/VisualRebuildV04/Validation/SourceDrift.json`. These are not missing runtime
assets. Reconcile these changes deliberately before any future complete source
regeneration; do not fold them into a validation-only resume.

## Evidence and recovery

- `Saved/VisualRebuildV04/Validation/`: audit, build, first/final regression logs,
  standalone validation, screenshots and comparison.
- `Saved/Checkpoints/Before_VisualRebuild_V04.tar.gz`: original pre-rebuild tree.
- `checkpoint-v04-playable-20260925`: owner's manually reviewed recovery state.
- `V04_PAUSED.md`: historical pause record, not active instructions.
