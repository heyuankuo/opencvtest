#pragma once

#include <windows.h>
//#undef MessageBox
 
enum QRESULT{ERR_OK,ERR_FEED_NUM,ERR_OPEN_QUE,ERR_CREATE_QUE,ERR_MEMORY_SIZE,ERR_ALREADY_EXISTS,ERR_CLOSING};
#define		MAX_BUFFERS			2
#define		MAX_FEEDS_NUM		128

struct GlobalHeader
{
	int		nScCounter;		//AddRef Release
	int		nReadBuffer;	//0,1 the buffer to be read now
	long	lImgSize;
	long    lQueueSize;
};

struct FrameHeader
{
	int		FrameNumber;
	int		nHeight;
	int		nWidth;
	int		uLsize; 
	int		XmlSize; 
	int		LayoutSize; 
	int		FileNameSize;
	int		NotUse2;
	int		NotUse1;
	int		IpPort;
	char    SystemIp[16];
	char    IntellexTimeStamp[64]; 
	BYTE	Bpp;
	BYTE	Format;// RGB = 0,YUV = 1...
	BYTE	SubFormat; 
	BYTE	CameraType;		// AXis=0, Sony=1, Intellex=2,File=3, PV=4, DVTel=5...
	BYTE	FeedInIntellex;
	BYTE	ByteInfo3; 
	BYTE	ByteInfo2; 
	BYTE	ByteInfo1; 
	char	FileName[256];
	char	NickName[256];
};

class HANDLE_CLASS
{
public:
	long  Event1, Event2;
//	HANDLE  Event[2];
	HANDLE_CLASS()
	{
		Event1 = NULL;
		Event2 = NULL;
	}
};
/*
CMemQueue:保存图像的内存队列空间类
*/
class CMemQueue
{
public:
	//Pointers to shared memory per feed.
	//HANDLE_CLASS		*m_DisplayFrameEvents[MAX_FEEDS_NUM+1];
	HANDLE_CLASS		hFillMutex[MAX_FEEDS_NUM+1];
	BYTE* 				m_pQueFeedNumDataPointers[MAX_FEEDS_NUM+1];
	HANDLE 				m_pQueFeedNumHandles[MAX_FEEDS_NUM+1];
 
	CMemQueue();
	
	~CMemQueue(void);
	 
	QRESULT		CloseQue(int nFeedNum);
	QRESULT		GetFrame(unsigned char* pFrame,int nFeedNum,FrameHeader*  pFrameHeader,bool bBlocking);
	QRESULT		SetFrame(unsigned char* pFrame,int nFeedNum,FrameHeader* stFrameHeader);
	
	//For scaners: just open it.
	//Open the que and init the frame header with the right
	//parameters for the scanner initialization (FrameHeader will be init from the reading headder position
	//gets from global header ).
	QRESULT		OpenQue(int nFeedNum,FrameHeader*  pFrameHeader);
	//For Mvs:	Create it...
	//Mvs will fill the FrameHeader struct with the camera params...
	QRESULT		CreateQue(int nFeedNum,FrameHeader  stFrameHeader, long	lImgSize);
	QRESULT		OpenEvents(int nFeedNum);

	 
};











