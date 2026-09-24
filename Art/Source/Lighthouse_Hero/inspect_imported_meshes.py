"""Print imported static mesh bounds/material slots while running as UE Python commandlet."""

import unreal

paths = (
    "/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_TowerShell.SM_BB_LH_TowerShell",
    "/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_Gallery.SM_BB_LH_Gallery",
    "/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_LanternRoom.SM_BB_LH_LanternRoom",
    "/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_AnnexDetails.SM_BB_LH_AnnexDetails",
    "/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_RockPlinth.SM_BB_LH_RockPlinth",
)
for path in paths:
    mesh = unreal.load_asset(path)
    if not mesh:
        unreal.log_error("Missing mesh: " + path)
        continue
    unreal.log("BB HERO MESH {} bounds {}".format(mesh.get_name(), mesh.get_bounds()))
    for slot in mesh.get_editor_property("static_materials"):
        material = slot.get_editor_property("material_interface")
        name = slot.get_editor_property("material_slot_name")
        unreal.log("BB HERO SLOT {} => {}".format(name, material.get_path_name() if material else "None"))
