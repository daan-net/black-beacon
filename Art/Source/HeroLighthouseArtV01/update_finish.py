"""Material-only correction after visual review; never reimports meshes."""
from pathlib import Path
import unreal as u
root=Path(__file__).resolve().parent
m=u.load_asset('/Game/BlackBeacon/Art/Lighthouse/Materials/M_Hero_ExteriorFinish')
for node in u.MaterialEditingLibrary.get_material_expressions(m):
    if node.get_editor_property('desc')=='weathering':
        node.set_editor_property('code',(root/'finish.hlsl').read_text())
u.MaterialEditingLibrary.recompile_material(m)
u.EditorAssetLibrary.save_loaded_asset(m)
