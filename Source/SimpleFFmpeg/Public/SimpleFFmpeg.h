// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class SIMPLEFFMPEG_API FSimpleFFmpegModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void MakeMp4(FString InMp4Path, FString InLogoPath);
	void ScanDirectory(TArray<FString>& Files, const FString& FilePath, const FString& Extension);
private:
	/** Handle to the test dll we will load */
	void*	ExampleLibraryHandle;
};
