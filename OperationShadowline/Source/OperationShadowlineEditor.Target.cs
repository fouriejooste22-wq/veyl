// Copyright (c) 2026 Operation Shadowline. Original IP. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OperationShadowlineEditorTarget : TargetRules
{
	public OperationShadowlineEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;

		ExtraModuleNames.Add("OperationShadowline");
	}
}
