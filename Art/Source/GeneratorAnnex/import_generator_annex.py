"""Run inside UE 5.8.2 editor to import the generated generator annex OBJ meshes."""

import os
import unreal

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../GeneratorAnnex/SourceMeshes"))
DESTINATION = "/Game/BlackBeacon/Art/GeneratorAnnex/Meshes"
MATERIAL_DESTINATION = "/Game/BlackBeacon/Art/GeneratorAnnex/Materials"

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
        ("TowerPaint", "M_GA_TowerPaint"),
        ("DarkIron", "M_GA_DarkIron"),
        ("WarmBrass", "M_GA_WarmBrass"),
        ("LanternGlass", "M_GA_LanternGlass"),
        ("AgedWood", "M_GA_AgedWood"),
        ("WetRock", "M_GA_WetRock")):
    source = DESTINATION + "/" + source_name
    target = MATERIAL_DESTINATION + "/" + target_name
    if unreal.EditorAssetLibrary.does_asset_exist(source):
        if unreal.EditorAssetLibrary.does_asset_exist(target):
            unreal.EditorAssetLibrary.delete_asset(target)
        unreal.EditorAssetLibrary.rename_asset(source, target)

unreal.EditorAssetLibrary.save_directory(DESTINATION, only_if_is_dirty=False, recursive=True)
unreal.EditorAssetLibrary.save_directory(MATERIAL_DESTINATION, only_if_is_dirty=False, recursive=True)
