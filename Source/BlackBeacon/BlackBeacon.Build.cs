using UnrealBuildTool;

public class BlackBeacon : ModuleRules
{
	public BlackBeacon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
		});

		// No PrivateDependencyModuleNames yet: keeping the dependency
		// surface minimal (rule: no unnecessary dependencies). Niagara is
		// intentionally NOT referenced until real effects assets exist (0.2+);
		// the weather/beam systems expose scalar hooks instead.
	}
}