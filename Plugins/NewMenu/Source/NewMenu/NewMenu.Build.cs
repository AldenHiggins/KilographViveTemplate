// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NewMenu : ModuleRules
{
    public NewMenu(TargetInfo Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"UnrealEd",
				"Slate",
                "SlateCore",
                "ImageWrapper",
                "AssetTools",
                "ContentBrowser"
            }
		);
		//Private Paths
        PrivateIncludePaths.AddRange(new[] { "NewMenu/Private" });
    }
}
