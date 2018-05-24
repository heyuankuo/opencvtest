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
#include <libavutil/avutil.h>  
#include <libswscale/swscale.h>  
#include <libavutil/imgutils.h>
}

using namespace std;

int main(int argc, char *argv[])
{
	ExceptionLog::ConfigLog(NULL, L"Lesson_2异常报告.txt");
	BOOL result = TRUE;
	int err = 0;

	//打开视频文件  
	FILE *in_file = nullptr;
	fopen_s(&in_file, "E://22.yuv", "rb");
	if (!in_file)
	{
		ExceptionLog::InputLog("fopen_s err");
		result = FALSE;
	}

	int in_w = 160, in_h = 120;
	int framenum = 50;
	const char* out_file = "E://22.mp4";

	// 过程起始位置
	AVFormatContext *pFormatCtx = nullptr;
	err = avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	if (err < 0)
	{
		ExceptionLog::InputLog("avformat_alloc_output_context2 err %d", err);
		result = FALSE;
	}
	else
	{
		AVOutputFormat *fmt = nullptr;
		fmt = pFormatCtx->oformat;
		err = avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE);
		if ( err < 0)
		{
			ExceptionLog::InputLog("avio_open err %d", err);
			result = FALSE;
		}
		else
		{
			AVStream *video_st = nullptr;
			video_st = avformat_new_stream(pFormatCtx, 0);
			if (nullptr == video_st)
			{
				ExceptionLog::InputLog("avformat_new_stream err %d");
				result = FALSE;
			}
			else
			{
				video_st->time_base = {1, 25};

				AVCodecContext *pCodecCtx = avcodec_alloc_context3(nullptr);
				if (nullptr == pCodecCtx)
				{
					ExceptionLog::InputLog("pCodecCtx 创建失败 err");
					result = FALSE;
				}
				else
				{
					avcodec_parameters_to_context(pCodecCtx, video_st->codecpar);
					pCodecCtx->codec_id = fmt->video_codec;
					pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
					pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
					pCodecCtx->width = in_w;
					pCodecCtx->height = in_h;
					pCodecCtx->time_base.num = 1;
					pCodecCtx->time_base.den = 25;
					pCodecCtx->bit_rate = 400000;
					pCodecCtx->gop_size = 10;

					if (pCodecCtx->codec_id == AV_CODEC_ID_H264)
					{
						pCodecCtx->qmin = 10;
						pCodecCtx->qmax = 51;
						pCodecCtx->qcompress = 0.6;
					}

					if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO)
						pCodecCtx->max_b_frames = 2;
					if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
						pCodecCtx->mb_decision = 2;

					AVCodec *pCodec = nullptr;
					pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
					if (!pCodec)
					{
						ExceptionLog::InputLog("avcodec_find_encoder err");
						result = FALSE;
					}
					else
					{
						err = avcodec_open2(pCodecCtx, pCodec, NULL);
						if (err < 0 )
						{
							ExceptionLog::InputLog("avcodec_open2 err");
							result = FALSE;
						}
						else
						{
							av_dump_format(pFormatCtx, 0, out_file, 1);

							AVFrame *picture = av_frame_alloc(); // 分配数据帧
							picture->width = pCodecCtx->width;
							picture->height = pCodecCtx->height;
							picture->format = pCodecCtx->pix_fmt;
							int size = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
							if (size < 1)
							{
								ExceptionLog::InputLog("av_image_get_buffer_size err %d", size);
								result = FALSE;
							}
							else
							{
								uint8_t *picture_buf = (uint8_t*)av_malloc(size);
								err = av_image_fill_arrays(picture->data, picture->linesize, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
								if (err < 0)
								{
									ExceptionLog::InputLog("av_image_fill_arrays err %d", err);
									result = FALSE;
								}
								else
								{
									err = avformat_write_header(pFormatCtx, NULL);

									AVPacket pkt = {0};
									int y_size = pCodecCtx->width * pCodecCtx->height;
									av_new_packet(&pkt, size * 3);

									for (int i = 0; i < framenum; i++)
									{
										//读入YUV  
										if (fread(picture_buf, 1, y_size * 3 / 2, in_file)<0)
										{
											ExceptionLog::InputLog("read file fail!");
											result = FALSE;
											break;
										}
										else if (feof(in_file))
											break;

										picture->data[0] = picture_buf; //亮度Y  
										picture->data[1] = picture_buf + y_size; //U  
										picture->data[2] = picture_buf + y_size * 5 / 4; //V  
																						 //AVFrame PTS  
										picture->pts = i;
										int got_picture = 0;

										//编码  
										int ret = avcodec_send_frame(pCodecCtx, picture);
										if (ret < 0)
										{
											ExceptionLog::InputLog("Error sending a frame for encoding");
											result = TRUE;
											break;
										}
										int ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);
										if (ret<0)
										{
											ExceptionLog::InputLog("encoder fail!");
											result = FALSE;
											break;
										}

										if (got_picture == 1)
										{
											cout << "encoder success!" << endl;

											// parpare packet for muxing  
											pkt.stream_index = video_st->index;
											av_packet_rescale_ts(&pkt, pCodecCtx->time_base, video_st->time_base);
											pkt.pos = -1;
											ret = av_interleaved_write_frame(pFormatCtx, &pkt);
											av_free_packet(&pkt);
										}
									}

									int ret = flush_encoder(pFormatCtx, 0);
									if (ret < 0)
									{
										cout << "flushing encoder failed!" << endl;
										goto end;
									}
									//[9]  

									//[10] --写文件尾  
									av_write_trailer(pFormatCtx);

								}
								av_free(picture_buf);
								av_free(picture);
							}
	
						}
					}
					
				}
			}
		}
		avio_close(pFormatCtx->pb);
		avformat_free_context(pFormatCtx);
	}
	
	return 0;
}