// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleFFmpeg.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include <string>
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
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
	int avcodec_get_context_defaults3(AVCodecContext* s, const AVCodec* codec);
	AVStream* avformat_new_stream(AVFormatContext* s, const AVCodec* c);
}
#define DATASIZE 2048*2048

AVStream* add_vidio_stream(AVFormatContext* oc, enum AVCodecID codec_id)//用以初始化一个用于输出的AVFormatContext结构体
{
	AVStream* st = nullptr;
	AVCodec* codec = nullptr;

	st = avformat_new_stream(oc, NULL);
	if (!st)
	{
		printf("Could not alloc stream\n");
		exit(1);
	}
	codec = avcodec_find_encoder(codec_id);//查找mjpeg解码器
	if (!codec)
	{
		printf("codec not found\n");
		exit(1);
	}
	avcodec_get_context_defaults3(st->codec, codec);//申请AVStream->codec(AVCodecContext对象)空间并设置默认值(由avcodec_get_context_defaults3()设置
	
	st->codec->bit_rate = 96;//设置采样参数，即比特率  
	st->codec->width = 1920;//设置视频宽高，这里跟图片的宽高保存一致即可
	st->codec->height = 1080;
	st->codec->time_base.den = 24;//设置帧率
	st->codec->time_base.num = 1;

	st->codec->pix_fmt = AV_PIX_FMT_RGBA64;//设置像素格式  
	st->codec->codec_tag = 0;
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)//一些格式需要视频流数据头分开
		st->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; 
	return st;
}

AVFrame* _video_format_convert(AVFrame* _frame_src, AVPixelFormat _new_fmt, int _new_width = 0, int _new_height = 0) {
	AVFrame* _frame_dest = av_frame_alloc();
	_frame_dest->width = (_new_width > 0 ? _new_width : _frame_src->width);
	_frame_dest->height = (_new_height > 0 ? _new_height : _frame_src->height);
	_frame_dest->format = _new_fmt;
	av_frame_get_buffer(_frame_dest, 0);
	// 初始化转码器
	SwsContext* _sws_ctx = sws_getContext(_frame_src->width, _frame_src->height, (AVPixelFormat)_frame_src->format, _frame_dest->width, _frame_dest->height, (AVPixelFormat)_frame_dest->format, SWS_BICUBIC, NULL, NULL, NULL);
	if (!_sws_ctx)
		printf("sws_getContext failed\n");
	// 转码
	sws_scale(_sws_ctx, (const uint8_t* const*)_frame_src->data, _frame_src->linesize, 0, _frame_src->height, _frame_dest->data, _frame_dest->linesize);
	sws_freeContext(_sws_ctx);
	return _frame_dest;
}

void FSimpleFFmpegModule::StartupModule()
{
	FString Test = avcodec_configuration();
	AVFormatContext* ofmt_ctx = NULL;//其包含码流参数较多，是一个贯穿始终的数据结构，很多函数都要用到它作为参数
	const char* out_filename = "d:\\a.avi";//输出文件路径，在这里也可以将mkv改成别的ffmpeg支持的格式，如mp4，flv，avi之类的
	int ret;//返回标志

	av_register_all();//初始化解码器和复用器
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);//初始化一个用于输出的AVFormatContext结构体，视频帧率和宽高在此函数里面设置
	if (!ofmt_ctx)
	{
		printf("Could not create output context\n");
		return;
	}

	//AVStream *out_stream = add_vidio_stream(ofmt_ctx, AV_CODEC_ID_MJPEG);//创造输出视频流
	AVStream* out_stream = add_vidio_stream(ofmt_ctx, AV_CODEC_ID_PNG);//创造输出视频流(第三个参数指向图片的格式)
	AVStream* Yuv_stream = add_vidio_stream(ofmt_ctx, AV_CODEC_ID_AYUV);
	av_dump_format(ofmt_ctx, 0, out_filename, 1);//该函数会打印出视频流的信息，如果看着不开心可以不要

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))//打开输出视频文件
	{
		ret = avio_open2(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE,nullptr,nullptr);
		if (ret < 0) {
			printf("Could not open output file '%s'", out_filename);
			return;
		}
	}

	if (avformat_write_header(ofmt_ctx, NULL) < 0)//写文件头（Write file header）
	{
		printf("Error occurred when opening output file\n");
		return;
	}

	int frame_index = 0;//放入视频的图像计数
	AVPacket* pkt = av_packet_alloc();
	av_init_packet(pkt);
	pkt->flags |= AV_PKT_FLAG_KEY;
	pkt->stream_index = out_stream->index;//获取视频信息，为压入帧图像做准备

	char buf[250] = { 0 };
	while (frame_index < 70)//将图像压入视频中
	{
		_snprintf(buf, 255, "MovieFrame00%03d.png", frame_index);//1_00000

		FString dir = "D:\\Projects\\NRClient\\WindowsNoEditor\\VRS\\Saved\\Screenshots\\WindowsNoEditor\\";
		FString filePath = dir + buf;
		TArray<uint8> OutArray;
		if (FFileHelper::LoadFileToArray(OutArray, *filePath))
		{
			pkt->size = OutArray.Num();
			pkt->data = OutArray.GetData();



			if (av_interleaved_write_frame(ofmt_ctx, pkt) < 0) //写入图像到视频
			{
				printf("Error muxing packet\n");
				break;
			}
			printf("Write %8d frames to output file\n", frame_index);//打印出当前压入的帧数
			frame_index++;
		}


		
	}


	av_free_packet(pkt);//释放掉帧数据包对象
	av_write_trailer(ofmt_ctx);//写文件尾（Write file trailer）
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);//关闭视频文件
	avformat_free_context(ofmt_ctx);//释放输出视频相关数据结构
}

void FSimpleFFmpegModule::ShutdownModule()
{
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleFFmpegModule, SimpleFFmpeg)
