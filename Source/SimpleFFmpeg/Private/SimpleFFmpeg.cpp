// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleFFmpeg.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
extern "C" {
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"

#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};
#define LOCTEXT_NAMESPACE "FSimpleFFmpegModule"

extern "C"
{
	const char* avcodec_configuration(void);
}

void FSimpleFFmpegModule::StartupModule()
{
	FString Test = avcodec_configuration();
	int32 i = 0;
}

void FSimpleFFmpegModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleFFmpegModule, SimpleFFmpeg)
