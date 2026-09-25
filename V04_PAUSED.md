# V0.4 pause history — superseded by resumed validation

The owner paused the first run because model quota was nearly exhausted.
Unfinished work was preserved as `071bba6`; the workspace was temporarily restored
to the previous storm version. These were historical recovery steps, not the
current project state.

The owner subsequently recovered the generated assets, manually tested them,
and committed `1c0cf71` on `wip/v04-visual-rebuild-20260925`, with immutable tag
`checkpoint-v04-playable-20260925`. The completed Vulkan build is recorded in
`Saved/VisualRebuildV04/AssetBuildRendered.log`.

Validation resumed from that exact checkpoint. Do not switch to main or repeat
the old full regeneration instructions. Read V04_RESUME.md and V04_VALIDATION.md.
The source/assets of the owner's recovery checkpoint remain recoverable through
the unchanged tag; the validation repair affects only gallery hatch clearance.
