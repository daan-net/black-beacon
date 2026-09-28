#!/usr/bin/env bash
# Start the installed game with cheaper rendering for slow or integrated GPUs.
# BB_PRESET=balanced (default) | fast | original. Extra arguments go to the game.
# Offscreen at 4K output on a 2-CU Radeon iGPU (Mesa RADV 26.1): original 2.3 fps,
# balanced ~15-19 fps (close to the original look), fast ~18-28 fps (softer, less rain).
set -euo pipefail

launcher="${BLACK_BEACON:-$HOME/.local/bin/black-beacon}"
if [[ ! -x "$launcher" ]]; then
    launcher="$(command -v black-beacon)" || {
        echo 'black-beacon launcher not found; install the game or set BLACK_BEACON=/path/to/launcher.' >&2
        exit 1
    }
fi

# Render for 1080p and upscale once to the display: 50% on 4K, 75% on 1440p.
height="${BB_DISPLAY_HEIGHT:-}"
if [[ -z "$height" ]]; then
    for connector in /sys/class/drm/card*-*; do
        [[ "$(cat "$connector/status" 2>/dev/null)" == connected ]] || continue
        mode="$(head -n1 "$connector/modes" 2>/dev/null)"
        height="${mode#*x}"
        break
    done
fi
secondary=100
if [[ "$height" =~ ^[0-9]+$ ]] && (( height > 1080 )); then
    secondary=$(( 108000 / height ))
fi

# test1-r2 crash fix (DefaultEngine.ini carries it for later builds).
cvars="r.PSOPrecache.GlobalShaders=0"
common="r.SecondaryScreenPercentage.GameViewport=$secondary,r.ScreenPercentage=50"
# Cloud shadows stay on: they darken the moonlight; without them the island washes out.
common+=",r.VolumetricCloud.ShadowMap.MaxResolution=256,r.VolumetricCloud.ShadowMap.RaySampleMaxCount=8"
common+=",r.VolumetricCloud.ShadowMap.RaySampleHorizonMultiplier=1,r.VolumetricCloud.ShadowMap.SpatialFiltering=0"
common+=",r.VolumetricCloud.ViewRaySampleMaxCount=96,r.VolumetricCloud.Shadow.ViewRaySampleMaxCount=24"
common+=",r.VolumetricCloud.EmptySpaceSkipping=1,r.VolumetricRenderTarget.Mode=0,r.VolumetricRenderTarget.Scale=0.5"
common+=",r.VolumetricFog.GridPixelSize=12,r.VolumetricFog.GridSizeZ=64"
common+=",r.Shadow.Virtual.Enable=0,r.Shadow.MaxCSMResolution=1024,r.Shadow.CSM.MaxCascades=2,r.Shadow.MaxResolution=1024"
common+=",r.AmbientOcclusionLevels=1,r.MotionBlurQuality=0,r.BloomQuality=3,r.TranslucencyLightingVolumeDim=32"
case "${BB_PRESET:-balanced}" in
    # TSR keeps the rain and thin railings; history at 100% instead of the scalability 200%.
    balanced)
        cvars+=",$common,r.AntiAliasingMethod=4,r.TSR.History.ScreenPercentage=100,r.TSR.History.UpdateQuality=0"
        cvars+=",r.TSR.RejectionAntiAliasingQuality=1,r.TSR.ShadingRejection.Flickering=0,r.TSR.History.R11G11B10=1,r.TSR.Resurrection=0"
        ;;
    # TAA is cheaper but blends most rain streaks away and is softer.
    fast) cvars+=",$common,r.AntiAliasingMethod=2,r.TemporalAA.Quality=1" ;;
    original) ;;
    *) echo "Unknown BB_PRESET '$BB_PRESET' (balanced, fast, original)." >&2; exit 1 ;;
esac

# Escape quits (test1-r2 has no exit binding; DefaultInput.ini adds one for later builds).
exec "$launcher" -dpcvars="$cvars" "-ExecCmds=setbind Escape quit" "$@"
