// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OperationShadowlineTarget : TargetRules
{
	public OperationShadowlineTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;

		ExtraModuleNames.Add("OperationShadowline");
	}
}
