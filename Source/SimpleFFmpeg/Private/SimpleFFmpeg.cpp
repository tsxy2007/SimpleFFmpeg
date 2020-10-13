#define _CRT_SECURE_NO_WARNINGS

#include "SimpleFFmpeg.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include <string>
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "libffmpeg/pch.h"
#include "string.h"

void FSimpleFFmpegModule::StartupModule()
{
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("SimpleFFmpeg"))->GetBaseDir();
	PluginDir += TEXT("/Binaries/ThirdParty/Win64/");
	TArray<FString> Files;
	ScanDirectory(Files, PluginDir, "*.dll");
	FPlatformProcess::PushDllDirectory(*PluginDir);
	for (int32 i = 0;i<Files.Num();i++)
	{
		FPlatformProcess::GetDllHandle(*Files[i]);
	}
	FPlatformProcess::PopDllDirectory(*PluginDir);
}

void FSimpleFFmpegModule::ShutdownModule()
{
}


void FSimpleFFmpegModule::MakeMp4(FString InMp4Path,FString InLogoPath)
{
	char* arrParams[14] = { 0 };
	for (int k = 0; k < 14; k++) {
		arrParams[k] = new char[512]();
	}
	std::string dir(TCHAR_TO_UTF8(*InMp4Path));
	std::string PngPath = dir + "MovieFrame00%03d.png";
	std::string Mp4Path = dir + "MovieOut.mp4";
	std::string LogoPath = TCHAR_TO_UTF8(*InLogoPath);
	strcpy_s(arrParams[0], strlen("ffmpeg") + 1, "ffmpeg");
	strcpy_s(arrParams[1], strlen("-i") + 1, "-i");
	strcpy_s(arrParams[2], strlen(PngPath.c_str()) + 1, PngPath.c_str());

	strcpy_s(arrParams[3], strlen("-i") + 1, "-i");
	strcpy_s(arrParams[4], strlen(LogoPath.c_str()) + 1, LogoPath.c_str());
	strcpy_s(arrParams[5], strlen("-filter_complex") + 1, "-filter_complex");
	strcpy_s(arrParams[6], strlen("overlay=20:20") + 1, "overlay=20:20");

	strcpy_s(arrParams[7], strlen("-c:v") + 1, "-c:v");
	strcpy_s(arrParams[8], strlen("libx264") + 1, "libx264");
	strcpy_s(arrParams[9], strlen("-r") + 1, "-r");
	strcpy_s(arrParams[10], strlen("24") + 1, "24");
	strcpy_s(arrParams[11], strlen("-pix_fmt") + 1, "-pix_fmt");
	strcpy_s(arrParams[12], strlen("yuv420p") + 1, "yuv420p");
	strcpy_s(arrParams[13], strlen(Mp4Path.c_str()) + 1, Mp4Path.c_str());

	//-i logo.png -filter_complex overlay
	ffmpeg_exec(14, arrParams);

	for (int k = 0; k < 14; k++)
	{
		delete[] arrParams[k];
	}
}

void FSimpleFFmpegModule::ScanDirectory(TArray<FString>& Files, const FString& FilePath, const FString& Extension)
{
	FString SearchedFiles = FilePath + Extension;
	TArray<FString> FindedFiles;

	IFileManager::Get().FindFiles(FindedFiles, *SearchedFiles, true, false);

	FString SearchFile = "";

	for (int i = 0; i < FindedFiles.Num(); i++)
	{
		SearchFile = FilePath + FindedFiles[i];
		Files.Add(SearchFile);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleFFmpegModule, SimpleFFmpeg)
