"""Audit lighthouse material assignments and texture import channel settings in UE."""

from pathlib import Path
import unreal

OUTPUT = Path("/tmp/blackbeacon_lighthouse_material_audit.txt")
materials = (
    "/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_TowerPaint.M_LH_TowerPaint",
    "/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_DarkIron.M_LH_DarkIron",
    "/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_WarmBrass.M_LH_WarmBrass",
    "/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_LanternGlass.M_LH_LanternGlass",
    "/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_AgedWood.M_LH_AgedWood",
    "/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_WetRock.M_LH_WetRock",
    "/Game/BlackBeacon/Materials/M_CoastSurface.M_CoastSurface",
    "/Game/BlackBeacon/Materials/M_WetBasaltRock.M_WetBasaltRock",
    "/Game/BlackBeacon/Materials/M_LanternLens.M_LanternLens",
)
textures = (
    "/Game/BlackBeacon/Textures/T_LighthousePaintAlbedo.T_LighthousePaintAlbedo",
    "/Game/BlackBeacon/Textures/T_WetBasaltAlbedo.T_WetBasaltAlbedo",
    "/Game/BlackBeacon/Textures/T_StormSkyPanorama.T_StormSkyPanorama",
)

lines = ["MATERIAL_API " + str([name for name in dir(unreal.MaterialEditingLibrary) if "material" in name.lower() or "expression" in name.lower() or "property" in name.lower()])]
for path in materials:
    asset = unreal.load_asset(path)
    lines.append("MATERIAL {} => {}".format(path, asset.get_class().get_name() if asset else "MISSING"))
    if not asset:
        continue
    for prop in ("blend_mode", "shading_model", "metallic", "specular", "roughness", "clear_coat", "two_sided"):
        try:
            lines.append("  {}={}".format(prop, asset.get_editor_property(prop)))
        except Exception:
            pass
    for prop in ("parent", "texture_parameter_values", "scalar_parameter_values", "vector_parameter_values"):
        try:
            value = asset.get_editor_property(prop)
            if value:
                lines.append("  {}={}".format(prop, value))
        except Exception:
            pass
    try:
        expressions = unreal.MaterialEditingLibrary.get_material_expressions(asset)
    except Exception:
        expressions = []
    if expressions:
        lines.append("  expression_count={}".format(len(expressions)))
    for expression in expressions or []:
        name = expression.get_class().get_name()
        details = []
        for prop in ("texture", "parameter_name", "r", "g", "b", "a", "value", "constant", "material_expression_editor_x", "material_expression_editor_y", "coordinate_index", "sampler_type"):
            try:
                value = expression.get_editor_property(prop)
            except Exception:
                continue
            if value is not None and str(value) not in ("", "0.0", "0"):
                if prop == "texture" and value:
                    try:
                        value = value.get_path_name()
                    except Exception:
                        pass
                details.append("{}={}".format(prop, value))
        inputs = []
        for prop in ("input", "a", "b", "base_color", "metallic", "specular", "roughness", "normal", "opacity", "clear_coat", "texture_object", "coordinates"):
            try:
                value = expression.get_editor_property(prop)
                connected = value.get_editor_property("expression")
                if connected:
                    inputs.append("{}<-{}".format(prop, connected.get_class().get_name()))
            except Exception:
                pass
        details.extend(inputs)
        lines.append("  EXPRESSION {} {}".format(name, " ".join(details)))

for path in textures:
    asset = unreal.load_asset(path)
    lines.append("TEXTURE {} => {}".format(path, asset.get_class().get_name() if asset else "MISSING"))
    if not asset:
        continue
    for prop in ("srgb", "compression_settings", "mip_gen_settings", "size_x", "size_y", "lod_group", "texture_group"):
        try:
            lines.append("  {}={}".format(prop, asset.get_editor_property(prop)))
        except Exception:
            pass

OUTPUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
unreal.log("Wrote lighthouse material audit to {}".format(OUTPUT))
