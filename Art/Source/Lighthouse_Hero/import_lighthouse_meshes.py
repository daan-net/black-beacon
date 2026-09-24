"""Run inside UE 5.8.2 editor to import the generated modular OBJ meshes."""

import os
import unreal

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../Lighthouse/SourceMeshes"))
DESTINATION = "/Game/BlackBeacon/Art/Lighthouse/Meshes"
MATERIAL_DESTINATION = "/Game/BlackBeacon/Art/Lighthouse/Materials"

tools = unreal.AssetToolsHelpers.get_asset_tools()
for filename in sorted(os.listdir(ROOT)):
    if not filename.lower().endswith(".obj"):
        continue
    task = unreal.AssetImportTask()
    task.filename = os.path.join(ROOT, filename)
    task.destination_path = DESTINATION
    task.automated = True
    task.replace_existing = True
    task.save = True
    tools.import_asset_tasks([task])
    unreal.log("BLACK BEACON imported {} -> {}".format(filename, task.imported_object_paths))

unreal.EditorAssetLibrary.make_directory(MATERIAL_DESTINATION)
for source_name, target_name in (
        ("TowerPaint", "M_LH_TowerPaint"),
        ("DarkIron", "M_LH_DarkIron"),
        ("WarmBrass", "M_LH_WarmBrass"),
        ("LanternGlass", "M_LH_LanternGlass"),
        ("AgedWood", "M_LH_AgedWood"),
        ("WetRock", "M_LH_WetRock")):
    source = DESTINATION + "/" + source_name
    target = MATERIAL_DESTINATION + "/" + target_name
    if unreal.EditorAssetLibrary.does_asset_exist(source):
        if unreal.EditorAssetLibrary.does_asset_exist(target):
            unreal.EditorAssetLibrary.delete_asset(target)
        unreal.EditorAssetLibrary.rename_asset(source, target)

unreal.EditorAssetLibrary.save_directory(DESTINATION, only_if_is_dirty=False, recursive=True)
unreal.EditorAssetLibrary.save_directory(MATERIAL_DESTINATION, only_if_is_dirty=False, recursive=True)
