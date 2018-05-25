// Lesson_2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <objbase.h>
#include "ExceptionLog.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

AVFormatContext *ifmt_ctx;
AVFormatContext *ofmt_ctx;

typedef struct StreamContext
{
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
} StreamContext;
StreamContext *stream_ctx;

BOOL g_efftstr[8] = { FALSE };

// 打开输入文件
int open_input_file(const char *filename)
{
	int ret;
	unsigned int i;

	ifmt_ctx = NULL;
	if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return ret;
	}

	stream_ctx = (StreamContext *)av_mallocz_array(ifmt_ctx->nb_streams, sizeof(StreamContext));
	if (!stream_ctx)
		return AVERROR(ENOMEM);

	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		AVStream *stream = ifmt_ctx->streams[i];
		AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
		AVCodecContext *codec_ctx;
		if (!dec)
		{
			av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
			continue;
			// return AVERROR_DECODER_NOT_FOUND;
		}
		codec_ctx = avcodec_alloc_context3(dec);
		if (!codec_ctx)
		{
			av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
			return AVERROR(ENOMEM);
		}
		ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
				"for stream #%u\n", i);
			return ret;
		}
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
				codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
			/* Open decoder */
			ret = avcodec_open2(codec_ctx, dec, NULL);
			if (ret < 0)
			{
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				return ret;
			}
		}
		stream_ctx[i].dec_ctx = codec_ctx;
		g_efftstr[i] = TRUE;
	}

	av_dump_format(ifmt_ctx, 0, filename, 0);
	return 0;
}

int open_output_file(const char *filename)
{
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;

	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
	if (!ofmt_ctx)
	{
		av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		return AVERROR_UNKNOWN;
	}

	AVStream *out_stream = NULL;
	AVStream *in_stream = NULL;
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (FALSE == g_efftstr[i])
			continue;

		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream)
		{
			av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
			return AVERROR_UNKNOWN;
		}

		in_stream = ifmt_ctx->streams[i];
		dec_ctx = stream_ctx[i].dec_ctx;

		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			/* in this example, we choose transcoding to same codec */
			encoder = avcodec_find_encoder(dec_ctx->codec_id);
			if (!encoder)
			{
				av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
				return AVERROR_INVALIDDATA;
			}
			enc_ctx = avcodec_alloc_context3(encoder);
			if (!enc_ctx)
			{
				av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
				return AVERROR(ENOMEM);
			}

			/* In this example, we transcode to same properties (picture size,
			* sample rate etc.). These properties can be changed for output
			* streams easily using filters */
			if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				enc_ctx->height = dec_ctx->height;
				enc_ctx->width = dec_ctx->width;
				enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
				/* take first format from list of supported formats */
				if (encoder->pix_fmts)
					enc_ctx->pix_fmt = encoder->pix_fmts[0];
				else
					enc_ctx->pix_fmt = dec_ctx->pix_fmt;
				/* video time_base can be set to whatever is handy and supported by encoder */
				enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
			}
			else
			{
				enc_ctx->sample_rate = dec_ctx->sample_rate;
				enc_ctx->channel_layout = dec_ctx->channel_layout;
				enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
				/* take first format from list of supported formats */
				enc_ctx->sample_fmt = encoder->sample_fmts[0];
				enc_ctx->time_base = { 1, enc_ctx->sample_rate };
			}

			/* Third parameter can be used to pass settings to encoder */
			ret = avcodec_open2(enc_ctx, encoder, NULL);
			if (ret < 0)
			{
				av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
				return ret;
			}
			ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
			if (ret < 0)
			{
				av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
				return ret;
			}
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

			out_stream->time_base = enc_ctx->time_base;
			stream_ctx[i].enc_ctx = enc_ctx;
		}
		else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN)
		{
			av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
			return AVERROR_INVALIDDATA;
		}
		else
		{
			/* if this stream must be remuxed */
			ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
			if (ret < 0)
			{
				av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
				return ret;
			}
			out_stream->time_base = in_stream->time_base;
		}

	}
	av_dump_format(ofmt_ctx, 0, filename, 1);

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
			return ret;
		}
	}

	/* init muxer, write output file header */
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
		return ret;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	ret = open_input_file("E:\\akiyo_qcif.yuv"); 
	ret = open_output_file("E:\\ceshi.mp4");

	AVPacket packet = { 0 };
	int stream_index = 0;
	enum AVMediaType type;
	AVFrame *frame = NULL;
	
	return 0;
}