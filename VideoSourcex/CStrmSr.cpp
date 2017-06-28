// CStrmSr.cpp : CCStrmSr 的实现

#include "stdafx.h"
#include "CStrmSr.h"
#include "memqueue.h"

// CCStrmSr


/**
 * 将VideoSource源显示在对应窗体上
 * 此函数为阻塞函数
 */
STDMETHODIMP CCStrmSr::PlayVideo(void)
{
	// TODO: 在此添加实现代码

	CMemQueue memqueue;
	FrameHeader frameHeader = {0};
	if( ERR_OPEN_QUE == memqueue.OpenQue(0, &frameHeader))
	{
		fprintf(stderr, "ERR_OPEN_QUE == memqueue.OpenQue(0, &frameHeader)\n");
		return 0x80070006; // 无效的句柄
	}

	IplImage *src		= cvCreateImageHeader( cvSize(frameHeader.nWidth, frameHeader.nHeight), IPL_DEPTH_8U, 3);
	IplImage *src_trdcolor	= cvCreateImage( cvSize(frameHeader.nWidth, frameHeader.nHeight), IPL_DEPTH_8U, 3);

	/*int sc = 2;
	IplImage *src_trdsize	= cvCreateImage( cvSize(frameHeader.nWidth * sc, frameHeader.nHeight* sc), IPL_DEPTH_8U, 3);*/

	// 获取桌面句柄
	HWND hwnd = GetDesktopWindow();
	hwnd = ::FindWindowEx(hwnd, NULL, L"Notepad", L"aa.txt - 记事本");
	hwnd = ::FindWindowEx(hwnd, NULL, L"Edit", L"");

	HDC hdc = GetWindowDC( hwnd );

	unsigned char * pFrame = (unsigned char *)malloc(3 * frameHeader.nWidth * frameHeader.nHeight);
	BITMAPINFOHEADER bmih ;
	bmih.biSize = sizeof (BITMAPINFOHEADER) ;
	bmih.biWidth = frameHeader.nWidth ;
	bmih.biHeight = frameHeader.nHeight ;
	bmih.biPlanes = 1 ;
	bmih.biBitCount = 24 ;
	bmih.biCompression = BI_RGB ;
	bmih.biSizeImage = 0 ;
	bmih.biXPelsPerMeter = 0 ;
	bmih.biYPelsPerMeter = 0 ;
	bmih.biClrUsed = 0 ;
	bmih.biClrImportant = 0 ;

	BYTE * pBits ;
	HBITMAP hBitmap = CreateDIBSection (NULL, (BITMAPINFO *) &bmih, 0, (void **)&pBits, NULL, 0) ;
	for(;;)
	{
		memset(pFrame, 0, 3 * frameHeader.nWidth * frameHeader.nHeight);
		memqueue.GetFrame(pFrame, 0, &frameHeader, false);

		src->imageData = (char *)pFrame;

		cvZero( src_trdcolor);
		cvCvtColor(src, src_trdcolor, CV_RGB2BGR);

		/*cvZero(src_trdsize);
		cvResize( src_trdcolor, src_trdsize );*/

		// cvShowImage("src", src_trdsize);

		memcpy_s(	(void *)pBits, 
					frameHeader.nWidth * frameHeader.nHeight *3, 
					src_trdcolor->imageData,
					frameHeader.nWidth * frameHeader.nHeight *3);
		
		HBRUSH brush= CreatePatternBrush(hBitmap);  
		::SelectObject( hdc, brush);

		RECT rect = {0, 0, 3 * frameHeader.nWidth, 3 * frameHeader.nHeight };
		FillRect( hdc, &rect, brush);

		DeleteObject( brush);

		if(-1 != cv::waitKey(40))
			break;
	}

	DeleteObject( hBitmap);
	ReleaseDC( hwnd, hdc);
	free( pFrame );
	// cvReleaseImage(&src_trdsize);
	cvReleaseImage(&src_trdcolor);
	_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF );  

	return S_OK;
}

STDMETHODIMP CCStrmSr::PlayVideoOnWnd(OLE_HANDLE hwnd)
{
	// TODO: 在此添加实现代码
	CMemQueue memqueue;
	FrameHeader frameHeader = {0};
	if( ERR_OPEN_QUE == memqueue.OpenQue(0, &frameHeader))
	{
		fprintf(stderr, "ERR_OPEN_QUE == memqueue.OpenQue(0, &frameHeader)\n");
		return 0x80070006; // 无效的句柄
	}

	HDC hdc = GetWindowDC( (HWND)hwnd );

	unsigned char * pFrame = (unsigned char *)malloc(3 * frameHeader.nWidth * frameHeader.nHeight);
	BITMAPINFOHEADER bmih ;
	bmih.biSize = sizeof (BITMAPINFOHEADER) ;
	bmih.biWidth = frameHeader.nWidth ;
	bmih.biHeight = frameHeader.nHeight ;
	bmih.biPlanes = 1 ;
	bmih.biBitCount = 24 ;
	bmih.biCompression = BI_RGB ;
	bmih.biSizeImage = 0 ;
	bmih.biXPelsPerMeter = 0 ;
	bmih.biYPelsPerMeter = 0 ;
	bmih.biClrUsed = 0 ;
	bmih.biClrImportant = 0 ;

	BYTE * pBits ;
	HBITMAP hBitmap = CreateDIBSection (NULL, (BITMAPINFO *) &bmih, 0, (void **)&pBits, NULL, 0) ;

	IplImage *src		= cvCreateImageHeader( cvSize(frameHeader.nWidth, frameHeader.nHeight), IPL_DEPTH_8U, 3);
	IplImage *src_trdcolor	= cvCreateImage( cvSize(frameHeader.nWidth, frameHeader.nHeight), IPL_DEPTH_8U, 3);
	IplImage *src_rota		= cvCreateImage( cvSize(frameHeader.nWidth, frameHeader.nHeight), IPL_DEPTH_8U, 3);
	for(;;)
	{
		memset(pFrame, 0, 3 * frameHeader.nWidth * frameHeader.nHeight);
		memqueue.GetFrame(pFrame, 0, &frameHeader, false);

		src->imageData = (char *)pFrame; //获取抓取的图像

		cvZero( src_trdcolor);
		cvCvtColor(src, src_trdcolor, CV_RGB2BGR); // 颜色模式转换

		cvZero( src_rota);
		cvFlip(src_trdcolor, src_rota, 0 ); // 镜像变换

		memcpy_s(	(void *)pBits, 
					frameHeader.nWidth * frameHeader.nHeight *3, 
					src_rota->imageData,
					frameHeader.nWidth * frameHeader.nHeight *3);
		
		HBRUSH brush= CreatePatternBrush(hBitmap);  
		::SelectObject( hdc, brush);

		RECT rect = {0, 0, frameHeader.nWidth, frameHeader.nHeight };
		FillRect( hdc, &rect, brush);
		DeleteObject( brush);

		if(-1 != cv::waitKey(40))
			break;
	}

	DeleteObject( hBitmap);
	ReleaseDC( (HWND)hwnd, hdc);
	free( pFrame );
	cvReleaseImage(&src_trdcolor);
	cvReleaseImage(&src_rota);

	return S_OK;
}

//逆时针旋转图像degree角度（原尺寸）
void CCStrmSr::RotateImage(IplImage* src, IplImage *dst, int degree)
{
	//旋转中心为图像中心
	CvPoint2D32f center;  
	center.x=float (src->width/2.0+0.5);
	center.y=float (src->height/2.0+0.5);

	//计算二维旋转的仿射变换矩阵
	float m[6];            
	CvMat M = cvMat( 2, 3, CV_32F, m );
	cv2DRotationMatrix( center, degree,1, &M);

	//变换图像，并用黑色填充其余值
	cvWarpAffine(src, dst, &M, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,cvScalarAll(0) );
}
