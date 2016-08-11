// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ScreenShots.generated.h"

/**
 * 
 */
UCLASS()
class VIVETEMPLATE_API UScreenShots : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Screenshot")
	static void TakeScreenShot();

private:

	// Helper functions to change the file location of the screenshots
	static TArray<FString> GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension);
	static UTexture2D* Victory_LoadTexture2D_FromFile(const FString& FullFilePath, bool& IsValid, int32& Width, int32& Height);
	static void SaveRenderTargetToDisk(UTexture2D* InRenderTarget, FString Filename);

};
