// Fill out your copyright notice in the Description page of Project Settings.

#include "ViveTemplate.h"

#include "ScreenShots.h"
#include "../ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
#include "HighResScreenshot.h"

void UScreenShots::TakeScreenShot()
{
	UE_LOG(LogTemp, Warning, TEXT("Moving all the current screenshots!!"));

	// Load up all of the screenshots currently in the directory
	FString directoryToSearch = FPaths::GameSavedDir() + TEXT("Screenshots/") + FPlatformProperties::PlatformName() + TEXT("/");
	FString filesStartingWith = TEXT("");
	FString fileExtensions = TEXT("png");

	// Search for all of the screenshots we've already saved
	TArray<FString> screenshotsAlreadySaved = GetAllFilesInDirectory(FPaths::GameDir() + TEXT("Screenshots/"), true, filesStartingWith, fileExtensions);

	TArray<FString> filesInDirectory = GetAllFilesInDirectory(directoryToSearch, true, filesStartingWith, fileExtensions);

	// Create a new screenshots directory if none exists
	FString screenshotsPath = FPaths::GameDir() + TEXT("Screenshots");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*screenshotsPath))
	{
		PlatformFile.CreateDirectory(*screenshotsPath);
	}

	// Load all of the screenshots into textures
	for (int fileIndex = 0; fileIndex < filesInDirectory.Num(); fileIndex++)
	{
		int width;
		int height;
		bool isValid;
		UTexture2D *loadedTexture = Victory_LoadTexture2D_FromFile(filesInDirectory[fileIndex], isValid, width, height);

		SaveRenderTargetToDisk(loadedTexture, FPaths::GameDir() + TEXT("Screenshots/Screenshot") + FString::FromInt(screenshotsAlreadySaved.Num()) + TEXT(".png"));

		UE_LOG(LogTemp, Warning, TEXT("Saved screenshot to disk"));
	}

	// Clear out the directory once we're done
	for (int fileIndex = 0; fileIndex < filesInDirectory.Num(); fileIndex++)
	{
		FString AbsoluteFilePath = filesInDirectory[fileIndex];
		if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*AbsoluteFilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't delete the screenshot"));
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Moving screenshots complete!"));
}

UTexture2D* UScreenShots::Victory_LoadTexture2D_FromFile(const FString& FullFilePath, bool& IsValid, int32& Width, int32& Height)
{
	IsValid = false;
	UTexture2D* LoadedT2D = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath))
	{
		return NULL;
	}

	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			//Valid?
			if (!LoadedT2D)
			{
				return NULL;
			}

			//Out!
			Width = ImageWrapper->GetWidth();
			Height = ImageWrapper->GetHeight();

			//Copy!
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			LoadedT2D->UpdateResource();
		}
	}

	// Success!
	IsValid = true;
	return LoadedT2D;
}

void UScreenShots::SaveRenderTargetToDisk(UTexture2D* InRenderTarget, FString Filename)
{
	// The array of colors of the texture that we're going to populate
	TArray<FColor> OutBMP;

	// Get ready to read the pixels of the texture
	FTexture2DMipMap* MyMipMap = &InRenderTarget->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MyMipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	uint8 PixelX = 5, PixelY = 10;
	uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
	FColor PixelColor;

	if (PixelX >= 0 && PixelX < TextureWidth && PixelY >= 0 && PixelY < TextureHeight)
	{
		PixelColor = FormatedImageData[PixelY * TextureWidth + PixelX];
	}

	// Add the texture pixels into outBMP
	for (uint32 heightIndex = 0; heightIndex < TextureHeight; heightIndex++)
	{
		for (uint32 widthIndex = 0; widthIndex < TextureWidth; widthIndex++)
		{
			OutBMP.Add(FormatedImageData[heightIndex * TextureWidth + widthIndex]);
		}
	}

	for (FColor& color : OutBMP)
	{
		color.A = 255;
	}

	FIntRect SourceRect;

	FIntPoint DestSize(InRenderTarget->GetSurfaceWidth(), InRenderTarget->GetSurfaceHeight());

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);

	// Unlock the texture data
	InRenderTarget->PlatformData->Mips[0].BulkData.Unlock();
}

/**
Gets all the files in a given directory.
@param directory The full path of the directory we want to iterate over.
@param fullpath Whether the returned list should be the full file paths or just the filenames.
@param onlyFilesStartingWith Will only return filenames starting with this string. Also applies onlyFilesEndingWith if specified.
@param onlyFilesEndingWith Will only return filenames ending with this string (it looks at the extension as well!). Also applies onlyFilesStartingWith if specified.
@return A list of files (including the extension).
*/
TArray<FString> UScreenShots::GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension)
{
	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
	PlatformFile.IterateDirectory(*directory, Visitor);
	TArray<FString> files;

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);
		bool shouldAddFile = true;

		// Check if filename starts with required characters
		if (!onlyFilesStartingWith.IsEmpty())
		{
			const FString left = fileName.Left(onlyFilesStartingWith.Len());

			if (!(fileName.Left(onlyFilesStartingWith.Len()).Equals(onlyFilesStartingWith)))
				shouldAddFile = false;
		}

		// Check if file extension is required characters
		if (!onlyFilesWithExtension.IsEmpty())
		{
			if (!(FPaths::GetExtension(fileName, false).Equals(onlyFilesWithExtension, ESearchCase::IgnoreCase)))
				shouldAddFile = false;
		}

		// Add full path to results
		if (shouldAddFile)
		{
			files.Add(fullPath ? filePath : fileName);
		}
	}

	return files;
}
