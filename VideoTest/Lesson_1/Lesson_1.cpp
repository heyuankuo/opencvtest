// Lesson_1.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ExceptionLog.h"

extern "C"
{
#include <libavcodec/avcodec.h>

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

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

static int encode_write(AVCodecContext *avctx, AVFrame *frame, FILE *fout)
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

		enc_pkt.stream_index = 0;
		ret = fwrite(enc_pkt.data, enc_pkt.size, 1, fout);
		av_packet_unref(&enc_pkt);
	}

end:
	ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
	return ret;
}

int main(int argc, char **argv)
{
	ExceptionLog::ConfigLog(NULL, L"Lesson_1异常报告.txt");
	const char *filename, *codec_name, *outfilename;
	const AVCodec *codec;
	AVCodecContext *c = NULL;
	int i, ret, x, y;
	FILE *f;
	AVFrame *frame;
	AVPacket *pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };


	if (argc <= 2)
	{
		ExceptionLog::InputLog("参数太少");
		fprintf(stderr, "Usage: %s <output file> <codec name>\n", argv[0]);
		exit(0);
	}
	filename = argv[1];
	codec_name = argv[2];

	/* find the mpeg1video encoder */
	codec = avcodec_find_encoder_by_name("libx264");
	if (!codec) 
	{
		ExceptionLog::InputLog("Codec '%s' not found", codec_name);
		fprintf(stderr, "Codec '%s' not found\n", codec_name);
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
	c->width = 176;
	c->height = 144;
	/* frames per second */
	c->time_base =  { 1, 25 };
	c->framerate =  { 25, 1 };

	/* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

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

	fopen_s(&f, filename, "wb");
	if (!f) 
	{
		ExceptionLog::InputLog("Could not open %s", filename);
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) 
	{
		ExceptionLog::InputLog("Could not allocate video frame");
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) 
	{
		ExceptionLog::InputLog("Could not allocate the video frame data");
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	/* encode 1 second of video */
	for (i = 0; i < 1000; i++) 
	{
		fflush(stdout);

		/* make sure the frame data is writable */
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			exit(1);

		/* prepare a dummy image */
		/* Y */
		for (y = 0; y < c->height; y++) 
		{
			for (x = 0; x < c->width; x++) 
			{
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}

		/* Cb and Cr */
		for (y = 0; y < c->height / 2; y++)
		{
			for (x = 0; x < c->width / 2; x++) 
			{
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}

		frame->pts = i;

		/* encode the image */
		encode_write(c, frame, f);
	}

	encode_write(c, NULL, f);

	// 添加结尾标记
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	system("pause");
	return 0;
}
