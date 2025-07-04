// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomMagicSense : ModuleRules
{
	public CustomMagicSense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "AIModule", "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
