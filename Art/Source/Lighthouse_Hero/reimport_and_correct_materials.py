"""Regenerate and reimport hero OBJ UV/material changes in UE 5.8.2."""

from pathlib import Path

root = Path(__file__).resolve().parent
for script_name in ("import_lighthouse_meshes.py", "apply_lighthouse_material_correction.py"):
    script_path = root / script_name
    exec(compile(script_path.read_text(encoding="utf-8"), str(script_path), "exec"), {
        "__file__": str(script_path), "__name__": "__main__"})
