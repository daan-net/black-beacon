"""Apply audited opaque PBR values to the generator annex materials.

Run with UE 5.8.2 editor Python after importing.
"""

from pathlib import Path
import unreal

ROOT = "/Game/BlackBeacon/Art/GeneratorAnnex/Materials"
COAST_MASTER = unreal.load_asset("/Game/BlackBeacon/Materials/M_CoastSurface")
BASALT_MASTER = unreal.load_asset("/Game/BlackBeacon/Materials/M_WetBasaltRock")

if not COAST_MASTER or not BASALT_MASTER:
    raise RuntimeError("Expected native PBR masters are missing")

# PBR properties: Metallic, Roughness, Specular, WetAmount, WetRoughness, WetDarkening, BaseColor
families = {
    "M_GA_WetConcrete": (0.0, 0.75, 0.2, 0.65, 0.40, 0.85, unreal.LinearColor(0.20, 0.21, 0.21, 1.0)),
    "M_GA_WetConcrete_Aged": (0.0, 0.80, 0.15, 0.45, 0.50, 0.88, unreal.LinearColor(0.18, 0.18, 0.17, 1.0)),
    "M_GA_Rust": (0.0, 0.95, 0.1, 0.20, 0.80, 0.95, unreal.LinearColor(0.25, 0.08, 0.03, 1.0)),
    "M_GA_OxidizedIron": (0.2, 0.75, 0.3, 0.30, 0.55, 0.90, unreal.LinearColor(0.08, 0.09, 0.10, 1.0)),
    "M_GA_ChippedPaint": (0.05, 0.65, 0.3, 0.25, 0.50, 0.90, unreal.LinearColor(0.06, 0.10, 0.08, 1.0)),
    "M_GA_DirtyGlass": (0.0, 0.55, 0.6, 0.10, 0.40, 0.95, unreal.LinearColor(0.03, 0.04, 0.04, 1.0)),
    "M_GA_OilyMetal": (0.75, 0.25, 0.8, 0.80, 0.15, 0.70, unreal.LinearColor(0.04, 0.04, 0.04, 1.0)),
    "M_GA_DampFloor": (0.0, 0.65, 0.1, 0.75, 0.30, 0.80, unreal.LinearColor(0.12, 0.13, 0.13, 1.0)),
    "M_GA_AgedWood": (0.0, 0.85, 0.1, 0.20, 0.55, 0.85, unreal.LinearColor(0.12, 0.08, 0.05, 1.0)),
    "M_GA_WarmBrass": (0.85, 0.35, 0.6, 0.10, 0.30, 0.90, unreal.LinearColor(0.35, 0.20, 0.06, 1.0)),
}

for name, (metallic_value, roughness_value, specular_value, wet_amount_value,
           wet_roughness_value, wet_darkening_value, color) in families.items():
    instance = unreal.load_asset(ROOT + "/" + name)
    if not instance:
        # Generate the instance if it doesn't exist
        factory = unreal.MaterialInstanceConstantFactoryNew()
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        instance = asset_tools.create_asset(name, ROOT, unreal.MaterialInstanceConstant, factory)
        
    if instance:
        unreal.MaterialEditingLibrary.clear_all_material_instance_parameters(instance)
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, COAST_MASTER)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            instance, "BaseColor", color)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "Metallic", metallic_value)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "Roughness", roughness_value)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "Specular", specular_value)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "WetAmount", wet_amount_value)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "WetRoughness", wet_roughness_value)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "WetDarkening", wet_darkening_value)
        unreal.MaterialEditingLibrary.update_material_instance(instance)
        unreal.EditorAssetLibrary.save_loaded_asset(instance)

unreal.log("Applied generator annex PBR materials.")
