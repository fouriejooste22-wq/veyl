// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

using UnrealBuildTool;

public class OperationShadowline : ModuleRules
{
	public OperationShadowline(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Kept deliberately minimal: only modules the current code actually references.
		// Later stages add their own (AIModule + NavigationSystem for stage 7,
		// ChaosVehicles for stage 8, MetasoundEngine for stage 5 audio, CommonUI and
		// GameplayTags for the stage 2 UI). Adding them before anything uses them only
		// lengthens the build and widens the surface for first-build errors.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RHI",				// GMaxRHIFeatureLevel, for the SM6 capability check
			"MotionWarping",	// traversal montages warped onto real geometry
			"UMG",
			"Slate",
			"SlateCore"
		});

		// PublicIncludePaths adds the module root, so headers are included as
		// "Player/ShadowlineCharacter.h" rather than with a long relative path.
		PublicIncludePaths.AddRange(new string[]
		{
			"OperationShadowline"
		});
	}
}
