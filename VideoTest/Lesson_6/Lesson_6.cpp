// Lesson_6.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <objbase.h>
#include <fstream>
#include "ExceptionLog.h" 

using std::ifstream;
using std::ofstream;
using std::ios_base;


// 创建输入格式上下文

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

SwsContext *sws_ctx = NULL;

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
	FILE *outfile)
{
	int ret;

	/* send the frame to the encoder */
	if (frame)
		printf("Send frame %3" PRId64 "\n", frame->pts);
	else
	{
		ExceptionLog::InputLog("空帧");
	}

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0)
	{
		ExceptionLog::InputLog("Error sending a frame for encoding");
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0)
	{
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0)
		{
			ExceptionLog::InputLog("Error during encoding");
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt); // 减少引用次数
	}
}

static int encode_write(AVCodecContext *avctx, AVFrame *frame, ofstream &ofs, AVCodecParserContext *parser)
{
	int ret = 0;
	AVPacket enc_pkt;

	av_init_packet(&enc_pkt);
	enc_pkt.data = NULL;
	enc_pkt.size = 0;

	if ((ret = avcodec_send_frame(avctx, frame)) < 0)
	{
		fprintf(stderr, "Error code: %s\n", av_err2str(ret));
		goto end;
	}
	while (1)
	{
		ret = avcodec_receive_packet(avctx, &enc_pkt);
		if (ret)
			break;

		// 重构每个包
		AVPacket *pkt = av_packet_alloc();
		av_init_packet(pkt);

		int datasize = enc_pkt.size;
		const uint8_t *data = enc_pkt.data;

		while (datasize > 0)
		{
			ret = av_parser_parse2(parser, avctx, &pkt->data, &pkt->size,
				data, datasize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

			data += ret;
			datasize -= ret;

			if (pkt->size)
				ofs.write((char *)pkt->data, pkt->size);
		}
			 
		av_packet_unref(&enc_pkt);

		av_packet_free(&pkt);
	}

end:
	ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
	return ret;
}

int main(int argc, char **argv)
{
	ExceptionLog::ConfigLog(NULL, L"Lesson_6异常报告.txt");
	const AVCodec *codec;
	AVCodecContext *c = NULL;
	int ret = 0;
	
	AVPacket *pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	ifstream ifs("TST.yuv", ios_base::binary);
	if (!ifs.is_open())
		return 0;

	ofstream ofs("E:\\TST_O_1.yuv", ios_base::binary | ios_base::ate);
	if (!ofs.is_open())
		return 0;

	sws_ctx = sws_getContext(	640, 480, AV_PIX_FMT_YUYV422,
								640, 480, AV_PIX_FMT_YUV422P,
								SWS_BILINEAR, NULL, NULL, NULL);

	/* find the mpeg1video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c)
	{
		ExceptionLog::InputLog("Could not allocate video codec context");
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	pkt = av_packet_alloc();
	if (!pkt)
	{
		ExceptionLog::InputLog("av_packet_alloc err");
		exit(1);
	}


	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = 640;
	c->height = 480;
	/* frames per second */
	c->time_base = { 1, 25 };
	c->framerate = { 25, 1 };

	/* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV422P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0)
	{
		ExceptionLog::InputLog("Could not open codec: %s", ret);
		fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}

	AVFrame *frame = av_frame_alloc();
	if (!frame)
	{
		ExceptionLog::InputLog("Could not allocate video frame");
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = AV_PIX_FMT_YUYV422;
	frame->width = 640;
	frame->height = 480;

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0)
	{
		ExceptionLog::InputLog("Could not allocate the video frame data");
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}


	AVFrame *frame_tm = av_frame_alloc();
	if (!frame_tm)
	{
		ExceptionLog::InputLog("Could not allocate video frame");
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame_tm->format = AV_PIX_FMT_YUV422P;
	frame_tm->width = 640;
	frame_tm->height = 480;

	ret = av_frame_get_buffer(frame_tm, 0);
	if (ret < 0)
	{
		ExceptionLog::InputLog("Could not allocate the video frame data");
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	AVCodecParserContext *parser = av_parser_init(codec->id);

	/* encode 1 second of video */
	for (int i = 0; i < 1000; i++)
	{
		fflush(stdout);

		/* make sure the frame data is writable */
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			exit(1);

		ifs.read((char *)frame->data[0], frame->linesize[0] * frame->height);

		frame->pts = i;

		sws_scale(sws_ctx, frame->data, frame->linesize, 0, 480, frame_tm->data, frame_tm->linesize);
		frame_tm->pts = i;

		/*ofs.write((char *)frame_tm->data[0], frame_tm->linesize[0] * frame_tm->height);
		ofs.write((char *)frame_tm->data[1], frame_tm->linesize[1] * frame_tm->height);
		ofs.write((char *)frame_tm->data[2], frame_tm->linesize[2] * frame_tm->height);*/

		/* encode the image */
		encode_write(c, frame_tm, ofs, parser);

		// av_frame_unref(&frame);
	}

	encode_write(c, NULL, ofs, parser);

	// 添加结尾标记
	ofs.write((char *)endcode, sizeof(endcode));

	ofs.clear();
	ifs.clear();

	ofs.close();
	ifs.close();

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	system("pause");
	return 0;
}

