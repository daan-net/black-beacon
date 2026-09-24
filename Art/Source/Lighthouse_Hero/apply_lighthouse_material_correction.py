"""Apply audited opaque PBR values to the imported lighthouse materials.

Run with UE 5.8.2 editor Python after importing generate_lighthouse_obj.py.
"""

from pathlib import Path
import unreal


ROOT = "/Game/BlackBeacon/Art/Lighthouse/Materials"
COAST_MASTER = unreal.load_asset("/Game/BlackBeacon/Materials/M_CoastSurface")
BASALT_MASTER = unreal.load_asset("/Game/BlackBeacon/Materials/M_WetBasaltRock")


def ensure_scalar_input(material, parameter, material_property):
    expressions = unreal.MaterialEditingLibrary.get_material_expressions(material)
    nodes = [expr for expr in expressions
             if expr.get_class().get_name() == "MaterialExpressionScalarParameter"
             and str(expr.get_editor_property("parameter_name")) == parameter]
    if nodes:
        # Keep the graph's existing nodes intact. UE roots material expressions
        # while compiling, and deleting a rooted expression can crash Editor.
        node = nodes[0]
    else:
        node = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionScalarParameter, 300, 520)
        node.set_editor_property("parameter_name", parameter)
        node.set_editor_property("default_value", 0.0 if parameter in ("Metallic", "Specular") else 0.8)
    if not unreal.MaterialEditingLibrary.connect_material_property(
            node, "", material_property):
        raise RuntimeError("Could not connect {} parameter to {}".format(parameter, material.get_name()))


def connect(source, destination, wanted_input):
    inputs = unreal.MaterialEditingLibrary.get_material_expression_input_names(destination)
    matching = [name for name in inputs if name.lower() == wanted_input.lower()]
    if not matching and len(inputs) == 1:
        matching = inputs
    if not matching or not unreal.MaterialEditingLibrary.connect_material_expressions(
            source, "", destination, matching[0]):
        raise RuntimeError("Could not connect {} to {}.{} ({})".format(
            source.get_class().get_name(), destination.get_class().get_name(), wanted_input, inputs))


def add_expression(material, expression_class, x, y, properties=None):
    expression = unreal.MaterialEditingLibrary.create_material_expression(
        material, expression_class, x, y)
    if not expression:
        raise RuntimeError("Could not create {}".format(expression_class))
    for name, value in (properties or {}).items():
        expression.set_editor_property(name, value)
    return expression


def ensure_local_wetness(material):
    expressions = unreal.MaterialEditingLibrary.get_material_expressions(material)
    if any(expr.get_class().get_name() == "MaterialExpressionScalarParameter"
           and str(expr.get_editor_property("parameter_name")) == "WetAmount"
           for expr in expressions):
        return

    base = unreal.MaterialEditingLibrary.get_material_property_input_node(
        material, unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.get_material_property_input_node(
        material, unreal.MaterialProperty.MP_ROUGHNESS)
    if not base or not roughness:
        raise RuntimeError("Expected BaseColor and Roughness inputs on {}".format(material.get_name()))

    world_position = add_expression(material, unreal.MaterialExpressionWorldPosition, 200, 900)
    object_position = add_expression(material, unreal.MaterialExpressionObjectPositionWS, 200, 1080)
    relative_position = add_expression(material, unreal.MaterialExpressionSubtract, 420, 940)
    connect(world_position, relative_position, "A")
    connect(object_position, relative_position, "B")

    height = add_expression(material, unreal.MaterialExpressionComponentMask, 640, 940,
                            {"b": True, "g": False, "r": False, "a": False})
    connect(relative_position, height, "Input")
    height_range = add_expression(material, unreal.MaterialExpressionScalarParameter, 640, 1120,
                                  {"parameter_name": "WetHeightCm", "default_value": 250.0})
    normalized_height = add_expression(material, unreal.MaterialExpressionDivide, 860, 940)
    connect(height, normalized_height, "A")
    connect(height_range, normalized_height, "B")
    lower_zone_inverse = add_expression(material, unreal.MaterialExpressionOneMinus, 1080, 940)
    connect(normalized_height, lower_zone_inverse, "Input")
    lower_zone = add_expression(material, unreal.MaterialExpressionSaturate, 1280, 940)
    connect(lower_zone_inverse, lower_zone, "Input")

    vertex_normal = add_expression(material, unreal.MaterialExpressionVertexNormalWS, 640, 1300)
    upward_facing = add_expression(material, unreal.MaterialExpressionComponentMask, 860, 1300,
                                   {"b": True, "g": False, "r": False, "a": False})
    connect(vertex_normal, upward_facing, "Input")
    upward_facing_saturated = add_expression(material, unreal.MaterialExpressionSaturate, 1080, 1300)
    connect(upward_facing, upward_facing_saturated, "Input")

    exposed_mask = add_expression(material, unreal.MaterialExpressionMax, 1500, 1040)
    connect(lower_zone, exposed_mask, "A")
    connect(upward_facing_saturated, exposed_mask, "B")
    wet_amount = add_expression(material, unreal.MaterialExpressionScalarParameter, 1500, 1260,
                                {"parameter_name": "WetAmount", "default_value": 0.35})
    wet_mask = add_expression(material, unreal.MaterialExpressionMultiply, 1720, 1040)
    connect(exposed_mask, wet_mask, "A")
    connect(wet_amount, wet_mask, "B")

    wet_roughness = add_expression(material, unreal.MaterialExpressionScalarParameter, 1500, 1440,
                                   {"parameter_name": "WetRoughness", "default_value": 0.48})
    roughness_blend = add_expression(material, unreal.MaterialExpressionLinearInterpolate, 1940, 900)
    connect(roughness, roughness_blend, "A")
    connect(wet_roughness, roughness_blend, "B")
    connect(wet_mask, roughness_blend, "Alpha")
    if not unreal.MaterialEditingLibrary.connect_material_property(
            roughness_blend, "", unreal.MaterialProperty.MP_ROUGHNESS):
        raise RuntimeError("Could not connect localized wet roughness")

    dry_multiplier = add_expression(material, unreal.MaterialExpressionConstant, 1500, 1620,
                                    {"r": 1.0})
    wet_multiplier = add_expression(material, unreal.MaterialExpressionScalarParameter, 1720, 1620,
                                    {"parameter_name": "WetDarkening", "default_value": 0.88})
    color_blend = add_expression(material, unreal.MaterialExpressionLinearInterpolate, 1940, 1260)
    connect(dry_multiplier, color_blend, "A")
    connect(wet_multiplier, color_blend, "B")
    connect(wet_mask, color_blend, "Alpha")
    darkened_color = add_expression(material, unreal.MaterialExpressionMultiply, 2160, 1100)
    connect(base, darkened_color, "A")
    connect(color_blend, darkened_color, "B")
    if not unreal.MaterialEditingLibrary.connect_material_property(
            darkened_color, "", unreal.MaterialProperty.MP_BASE_COLOR):
        raise RuntimeError("Could not connect localized wet color")


if not COAST_MASTER or not BASALT_MASTER:
    raise RuntimeError("Expected native PBR masters are missing")

# The authored whitewash and basalt maps are color data, not mask/normal maps.
for asset_path in (
        "/Game/BlackBeacon/Textures/T_LighthousePaintAlbedo",
        "/Game/BlackBeacon/Textures/T_WetBasaltAlbedo"):
    texture = unreal.load_asset(asset_path)
    if not texture:
        raise RuntimeError("Missing texture {}".format(asset_path))
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)

for master in (COAST_MASTER, BASALT_MASTER):
    ensure_scalar_input(master, "Metallic", unreal.MaterialProperty.MP_METALLIC)
    ensure_scalar_input(master, "Specular", unreal.MaterialProperty.MP_SPECULAR)
    ensure_local_wetness(master)
    unreal.MaterialEditingLibrary.recompile_material(master)
    unreal.EditorAssetLibrary.save_loaded_asset(master)

# Interchange's generated Phong parent had high Ks/Ns values. Reparent these
# instances to the project's opaque PBR master, then set each physical family.
families = {
    "M_LH_TowerPaint": (0.0, 0.84, 0.27, 0.35, 0.48, 0.88, unreal.LinearColor(0.72, 0.75, 0.78, 1.0)),
    "M_LH_DarkIron": (0.12, 0.72, 0.28, 0.22, 0.50, 0.90, unreal.LinearColor(0.075, 0.088, 0.092, 1.0)),
    "M_LH_WarmBrass": (0.82, 0.43, 0.50, 0.12, 0.40, 0.94, unreal.LinearColor(0.42, 0.25, 0.09, 1.0)),
    # Non-lantern windows stay opaque and subdued; only the controller's
    # lantern glazing uses the separate luminous pane material.
    "M_LH_LanternGlass": (0.0, 0.58, 0.32, 0.03, 0.45, 0.95, unreal.LinearColor(0.025, 0.04, 0.05, 1.0)),
    "M_LH_AgedWood": (0.0, 0.76, 0.27, 0.15, 0.50, 0.88, unreal.LinearColor(0.17, 0.105, 0.065, 1.0)),
    "M_LH_WetRock": (0.0, 0.62, 0.30, 0.65, 0.38, 0.82, unreal.LinearColor(0.075, 0.087, 0.09, 1.0)),
}

for name, (metallic_value, roughness_value, specular_value, wet_amount_value,
           wet_roughness_value, wet_darkening_value, color) in families.items():
    instance = unreal.load_asset(ROOT + "/" + name)
    if not instance:
        raise RuntimeError("Missing imported lighthouse material {}".format(name))
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

unreal.log("Applied opaque lighthouse PBR materials and corrected color texture import settings")
