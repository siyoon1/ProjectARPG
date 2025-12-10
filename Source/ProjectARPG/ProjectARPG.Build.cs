// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectARPG : ModuleRules
{
	public ProjectARPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"NavigationSystem", "UMG", "Niagara", "CableComponent"});
	}
}
