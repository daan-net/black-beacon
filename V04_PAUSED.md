# V0.4 paused — resume after usage reset

Active branch: **main**, prior verified playable storm version (`7bc4c0a`).
Unfinished rebuild: **wip/v04-visual-rebuild-20260925**, commit **071bba6**.

The editor and shader workers used for V0.4 were stopped before restoring main.
Main's Unreal module is rebuilt to match its source and assets. No V0.4 geometry
or shader changes are active on main. Original user texture edits and eight
untracked reference PNGs remain untouched.

## Resume

Switch to `wip/v04-visual-rebuild-20260925`, preserving any later user changes.
Read that branch's `V04_RESUME.md` and `Art/Source/VisualRebuildV04/README.md`.
Detailed instructions are also copied to `Saved/VisualRebuildV04/ResumeDetails.md`.

V0.4 source is substantial, but not accepted: C++ builds; logic/geometry checks
pass; Vulkan compilation was interrupted; integrated gameplay and A–O captures
have not run. Regenerate final source geometry, complete UE asset building and
section audit, build, then run the single integrated review suite as documented.

## Backups

- `checkpoint/pre-v04-20260925`: original committed source.
- `Saved/Checkpoints/Before_VisualRebuild_V04.tar.gz`: full original working tree,
  including the user's existing texture modifications and supplied references.
- `Saved/Checkpoints/Paused_VisualRebuild_V04_SourceAssets.tar.gz`: deterministic
  Git archive of the unfinished V0.4 commit; excludes user-only modifications.
- `Saved/VisualRebuildV04/Before/`: previous rendered screenshots.
- `Saved/VisualRebuildV04/UnusedImports/`: intermediate OBJ-import materials/cache,
  moved out of active Content; not deliverable assets.

Use the branch/Git archive as the authoritative unfinished source checkpoint.
Do not extract backups over new user edits. No V0.4 milestone completion claimed.
