// Lesson_4.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <objbase.h>
#include "ExceptionLog.h"
#include <fstream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil\imgutils.h>
}

using std::ofstream;
using std::ios_base;

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
typedef struct FilteringContext {
	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
} FilteringContext;
static FilteringContext *filter_ctx;

typedef struct StreamContext {
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
} StreamContext;
static StreamContext *stream_ctx;

BOOL effcStream[8] = { FALSE };

static int open_input_file(const char *filename)
{
	int ret;
	unsigned int i;

	ifmt_ctx = NULL;
	if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return ret;
	}

	stream_ctx = (StreamContext *)av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
	if (!stream_ctx)
		return AVERROR(ENOMEM);

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
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
		if (!codec_ctx) {
			av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
			return AVERROR(ENOMEM);
		}
		ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
				"for stream #%u\n", i);
			return ret;
		}
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
				codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
			/* Open decoder */
			ret = avcodec_open2(codec_ctx, dec, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				return ret;
			}
		}
		stream_ctx[i].dec_ctx = codec_ctx;
		effcStream[i] = TRUE;
	}

	av_dump_format(ifmt_ctx, 0, filename, 0);
	return 0;
}

static int open_output_file(const char *filename)
{
	AVStream *out_stream;
	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;

	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
	if (!ofmt_ctx) {
		av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		return AVERROR_UNKNOWN;
	}


	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (FALSE == effcStream[i])
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
			else {
				enc_ctx->sample_rate = dec_ctx->sample_rate;
				enc_ctx->channel_layout = dec_ctx->channel_layout;
				enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
				/* take first format from list of supported formats */
				enc_ctx->sample_fmt = encoder->sample_fmts[0];
				enc_ctx->time_base = { 1, enc_ctx->sample_rate };
			}

			/* Third parameter can be used to pass settings to encoder */
			ret = avcodec_open2(enc_ctx, encoder, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
				return ret;
			}
			ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
			if (ret < 0) {
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
			if (ret < 0) {
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
		if (ret < 0) {
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

static int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,
	AVCodecContext *enc_ctx, const char *filter_spec)
{
	char args[512];
	int ret = 0;
	const AVFilter *buffersrc = NULL;
	const AVFilter *buffersink = NULL;
	AVFilterContext *buffersrc_ctx = NULL;
	AVFilterContext *buffersink_ctx = NULL;
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	AVFilterGraph *filter_graph = avfilter_graph_alloc();

	if (!outputs || !inputs || !filter_graph) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		buffersrc = avfilter_get_by_name("buffer");
		buffersink = avfilter_get_by_name("buffersink");
		if (!buffersrc || !buffersink) {
			av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
			dec_ctx->time_base.num, dec_ctx->time_base.den,
			dec_ctx->sample_aspect_ratio.num,
			dec_ctx->sample_aspect_ratio.den);

		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
			goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
			(uint8_t*)&enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
			goto end;
		}
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		buffersrc = avfilter_get_by_name("abuffer");
		buffersink = avfilter_get_by_name("abuffersink");
		if (!buffersrc || !buffersink) {
			av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		if (!dec_ctx->channel_layout)
			dec_ctx->channel_layout =
			av_get_default_channel_layout(dec_ctx->channels);
		snprintf(args, sizeof(args),
			"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
			dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
			av_get_sample_fmt_name(dec_ctx->sample_fmt),
			dec_ctx->channel_layout);
		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
			goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
			(uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
			(uint8_t*)&enc_ctx->channel_layout,
			sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
			(uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
			goto end;
		}
	}
	else {
		ret = AVERROR_UNKNOWN;
		goto end;
	}

	/* Endpoints for the filter graph. */
	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	if (!outputs->name || !inputs->name) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
		&inputs, &outputs, NULL)) < 0)
		goto end;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		goto end;

	/* Fill FilteringContext */
	fctx->buffersrc_ctx = buffersrc_ctx;
	fctx->buffersink_ctx = buffersink_ctx;
	fctx->filter_graph = filter_graph;

end:
	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	return ret;
}

static int init_filters(void)
{
	const char *filter_spec;
	unsigned int i;
	int ret;
	filter_ctx = (FilteringContext *)av_malloc_array(ifmt_ctx->nb_streams, sizeof(*filter_ctx));
	if (!filter_ctx)
		return AVERROR(ENOMEM);

	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (FALSE == effcStream[i])
			continue;

		filter_ctx[i].buffersrc_ctx = NULL;
		filter_ctx[i].buffersink_ctx = NULL;
		filter_ctx[i].filter_graph = NULL;
		if (!(ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
			|| ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO))
			continue;


		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			filter_spec = "null"; /* passthrough (dummy) filter for video */
		else
			filter_spec = "anull"; /* passthrough (dummy) filter for audio */
		ret = init_filter(&filter_ctx[i], stream_ctx[i].dec_ctx,
			stream_ctx[i].enc_ctx, filter_spec);
		if (ret)
			return ret;
	}
	return 0;
}

// 编码
static int encode_write(AVFrame *frame, unsigned int stream_index)
{
	int ret = 0;
	AVPacket enc_pkt;

	enc_pkt.data = NULL;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);

	if ((ret = avcodec_send_frame(stream_ctx[stream_index].enc_ctx, frame)) < 0)
	{
		fprintf(stderr, "Error during encoding. Error code: %s\n", av_err2str(ret));
		goto end;
	}
	while (1)
	{
		ret = avcodec_receive_packet(stream_ctx[stream_index].enc_ctx, &enc_pkt);
		if (ret)
			break;

		enc_pkt.stream_index = stream_index;
		av_packet_rescale_ts(&enc_pkt,
			stream_ctx[stream_index].enc_ctx->time_base,
			ofmt_ctx->streams[stream_index]->time_base);

		av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
		ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
		if (ret < 0)
		{
			fprintf(stderr, "Error during writing data to output file. "
				"Error code: %s\n", av_err2str(ret));
			return -1;
		}
	}

	av_frame_unref(frame);

end:
	if (ret == AVERROR_EOF)
		return 0;
	ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
	return ret;
}

static int filter_encode_write_frame(AVFrame *frame, unsigned int stream_index)
{
	int ret;
	AVFrame *filt_frame;

	av_log(NULL, AV_LOG_INFO, "Pushing decoded frame to filters\n");
	/* push the decoded frame into the filtergraph */
	ret = av_buffersrc_add_frame_flags(filter_ctx[stream_index].buffersrc_ctx, frame, 0);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
		return ret;
	}

	/* pull filtered frames from the filtergraph */
	while (1)
	{
		filt_frame = av_frame_alloc();
		if (!filt_frame) {
			ret = AVERROR(ENOMEM);
			break;
		}
		av_log(NULL, AV_LOG_INFO, "Pulling filtered frame from filters\n");
		ret = av_buffersink_get_frame(filter_ctx[stream_index].buffersink_ctx, filt_frame);
		if (ret < 0)
		{
			/* if no more frames for output - returns AVERROR(EAGAIN)
			* if flushed and no more frames for output - returns AVERROR_EOF
			* rewrite retcode to 0 to show it as normal procedure completion
			*/
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				ret = 0;
			av_frame_free(&filt_frame);
			break;
		}

		filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
		// ret = encode_write_frame(filt_frame, stream_index, NULL);
		ret = encode_write(filt_frame, stream_index);
		if (ret < 0)
			break;
	}

	return ret;
}

static int flush_encoder(unsigned int stream_index)
{
	int ret;
	int got_frame = 0;

	if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
		AV_CODEC_CAP_DELAY))
		return 0;

	while (1) {
		av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
		// ret = encode_write_frame(NULL, stream_index, &got_frame);
		ret = encode_write(NULL, stream_index);
		if (ret < 0)
			break;
		if (!got_frame)
			return 0;
	}
	return ret;
}

// 解码
static int decode_packet(const AVPacket *pkt, unsigned int stream_index, ofstream &ofs)
{
	int ret = avcodec_send_packet(stream_ctx[stream_index].dec_ctx, pkt);
	if (ret < 0)
	{
		fprintf(stderr, "Error while sending a packet to the decoder: %s\n", av_err2str(ret));
		return ret;
	}

	while (ret >= 0)
	{
		AVFrame *frame = av_frame_alloc();
		if (!frame)
		{
			fprintf(stderr, "Can not alloc frame\n");
			ret = AVERROR(ENOMEM);
			return ret;
		}

		ret = avcodec_receive_frame(stream_ctx[stream_index].dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			break;
		}
		else if (ret < 0)
		{
			fprintf(stderr, "Error while receiving a frame from the decoder: %s\n", av_err2str(ret));
			return ret;
		}
		else // 解码成功
		{
			// 输出一段YUV裸流
			ofs.write((const char *)frame->data[0], frame->linesize[0] * frame->height);
			ofs.write((const char *)frame->data[1], frame->linesize[1] * frame->height / 2);
			ofs.write((const char *)frame->data[2], frame->linesize[2] * frame->height / 2);

			frame->pts = frame->best_effort_timestamp;
			ret = filter_encode_write_frame(frame, stream_index);
			av_frame_unref(frame);
			if (ret < 0) // 编码异常
			{
				fprintf(stderr, "filter_encode_write_frame err: %s\n", av_err2str(ret));
				return ret;
			}
		}
	}

	return 0;
}

// 编码
static int encode_write(AVFrame *frame, AVFormatContext *pofmt_ctx, AVCodecContext *enc_ctx, AVBSFContext *bsf_ctx)
{
	int ret = 0;
	AVPacket enc_pkt;

	enc_pkt.data = NULL;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);

	if ((ret = avcodec_send_frame(enc_ctx, frame)) < 0)
	{
		fprintf(stderr, "Error during encoding. Error code: %s\n", av_err2str(ret));
		goto end;
	}

	while (1)
	{
		ret = avcodec_receive_packet(enc_ctx, &enc_pkt);
		if (ret)
			break;

		ret = av_bsf_send_packet(bsf_ctx, &enc_pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return 0;

		ret = av_bsf_receive_packet(bsf_ctx, &enc_pkt);

		av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base, pofmt_ctx->streams[0]->time_base);

		av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
		ret = av_interleaved_write_frame(pofmt_ctx, &enc_pkt);
		if (ret < 0)
		{
			fprintf(stderr, "Error during writing data to output file. "
				"Error code: %s\n", av_err2str(ret));
			return -1;
		}
	}

	// av_frame_unref(frame);

end:
	if (ret == AVERROR_EOF)
		return 0;
	ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
	return ret;
}

int main(int argc, char **argv)
{
	//打开视频  
	FILE *in_file = NULL;
	fopen_s(&in_file, "E:\\akiyo_qcif.yuv", "rb");

	int in_w = 176, in_h = 144;
	int framenum = 300;

	int ret = 0;

	// 根据文件名推断合适的封装格式（.mp4）
	const char* out_file = "E:\\src01.mp4";
	AVFormatContext *pFormatCtx = NULL;
	ret = avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	
	// 打开输出文件
	ret = avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE);

	// 查找编码器
	AVCodec *pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);

	// 创建编码上下文
	AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->gop_size = 10;

	// 创建输出流
	AVStream *video_st = avformat_new_stream(pFormatCtx, pCodec);
	ret = avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);
	video_st->time_base.num = pCodecCtx->time_base.num;
	video_st->time_base.den = pCodecCtx->time_base.den;

	AVDictionary* opt = NULL;
	av_dict_set_int(&opt, "video_track_timescale", 25, 0);
	ret = avcodec_open2(pCodecCtx, pCodec, &opt);

	//输出格式信息 
	av_dump_format(pFormatCtx, 0, out_file, 1);

	// 写头文件  
	ret = avformat_write_header(pFormatCtx, &opt);

	AVFrame *picture = av_frame_alloc();
	picture->width = pCodecCtx->width;
	picture->height = pCodecCtx->height;
	picture->format = pCodecCtx->pix_fmt;
	ret = av_frame_get_buffer(picture, 0);

	AVBSFContext *bsf_ctx = NULL;
	const AVBitStreamFilter *filter = av_bsf_get_by_name("h264_mp4toannexb");
	ret = av_bsf_alloc(filter, &bsf_ctx);

	for (int i = 0; i < framenum; i++)
	{
		
		fread(picture->data[0], 1, picture->linesize[0]* picture->height, in_file);
		fread(picture->data[1], 1, picture->linesize[1] * picture->height / 2, in_file);
		fread(picture->data[2], 1, picture->linesize[2] * picture->height / 2, in_file);
		picture->pts = i;

		encode_write(picture, pFormatCtx, pCodecCtx, bsf_ctx);
	}

	encode_write(NULL, pFormatCtx, pCodecCtx, bsf_ctx);
	av_write_trailer(pFormatCtx);

	fclose(in_file);
}