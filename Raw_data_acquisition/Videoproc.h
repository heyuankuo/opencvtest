#pragma once

class CVideoproc
{
public:
	CVideoproc(void);
	~CVideoproc(void);

	/**
	 * �˺�������¼�� �� Ԥ����Ƶ
	 * ע���˺���Ϊ��������
	 * @param			videoname		[in]		�������Ƶ����
	 * @param			preview			[in]		Ԥ���������
	 */
	void RecordVideo( const char *videoname, const char *preview);

	/**
	 * ���֡��Ϣ
	 * ��ÿ֡�ϱ��գ�۲���������ܻ�������ͼ
	 */
	void MarkVideo( const char *videoname );
};
