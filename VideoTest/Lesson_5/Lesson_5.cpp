// Lesson_5.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <objbase.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil\imgutils.h>
#include <libavdevice\avdevice.h>
}

AVPixelFormat xx;

byte *tmp_buffer = NULL;
byte *tmp_buffer = NULL; // Y
byte *tmp_buffer = NULL; // U
byte *tmp_buffer = NULL; // V

int main()
{
	tmp_buffer = (byte *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1280 * 480);



    return 0;
}

