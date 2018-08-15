// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Chess : ModuleRules
{
	public Chess(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
		    "Core", "CoreUObject", 
		    "Engine", "InputCore",
		    "ApexDestruction", "OnlineSubsystem", 
		    "OnlineSubsystemUtils" });
        
	    DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
