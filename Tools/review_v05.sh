#!/usr/bin/env bash
# V0.5 integrated gameplay regression and native 1080p review captures.
set -euo pipefail
repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
engine_dir="${UE_ROOT:-$HOME/UnrealEngine}"
"$engine_dir/Engine/Binaries/Linux/UnrealEditor" "$repo_dir/BlackBeacon.uproject" \
    -game -unattended -vulkan -RenderOffscreen -windowed -ForceRes -ResX=1920 -ResY=1080 \
    -nosplash -stdout '-ExecCmds=r.SetRes 1920x1080w,r.DynamicRes.OperationMode 0,r.ScreenPercentage 100,Automation RunTests BlackBeacon; Quit' \
    "-ReportExportPath=$repo_dir/Saved/Automation/VisualRebuildV05Review"
