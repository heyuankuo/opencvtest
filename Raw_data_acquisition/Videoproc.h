#pragma once

class CVideoproc
{
public:
	CVideoproc(void);
	~CVideoproc(void);

	/**
	 * 此函数用于录制 和 预览视频
	 * 注：此函数为阻塞函数
	 * @param			videoname		[in]		保存的视频名称
	 * @param			preview			[in]		预览窗体标题
	 */
	void RecordVideo( const char *videoname, const char *preview);

	/**
	 * 标记帧信息
	 * 在每帧上标记眨眼参数，最好能绘制折线图
	 */
	void MarkVideo( const char *videoname );
};
