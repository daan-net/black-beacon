using UnrealBuildTool;
using System.Collections.Generic;

public class BlackBeaconEditorTarget : TargetRules
{
	public BlackBeaconEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("BlackBeacon");
	}
}