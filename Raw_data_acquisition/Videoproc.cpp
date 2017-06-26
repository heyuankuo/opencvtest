#include "Videoproc.h"
#include "opencvref.h"

#include "SunLightFace.h"

#pragma comment(lib, "SunLightFace.lib")

CVideoproc::CVideoproc(void)
{
}

CVideoproc::~CVideoproc(void)
{
}

/**
 * 此函数用于录制 和 预览视频
 * 注：此函数为阻塞函数
 * @param			videoname		[in]		保存的视频名称
 * @param			preview			[in]		预览窗体标题
 */
void CVideoproc::RecordVideo( const char *videoname,
								const char *preview)
{ 

	//获取默认摄像头，此处应有检测
	CvCapture* pCapture = cvCreateCameraCapture(0); 

    //创建并设置预览窗口对象 
    cvNamedWindow( preview, 1); 

	// 设置视频属性
    int isColor = 1;  
    int fps = 30; 
    int frameW = 640; 
    int frameH = 480;  
	CvVideoWriter *writer = cvCreateVideoWriter(videoname,CV_FOURCC('X','V','I','D'),fps,cvSize(frameW,frameH),isColor);  
  
	IplImage* pFrame = NULL;  // 视频帧 指针缓存
	IplImage* img   = NULL;	// 图像数据指针缓存
    while(1)  
    {  
        pFrame=cvQueryFrame( pCapture );  // 从视频源锁定一帧图像
        if(!pFrame)break; 

        cvShowImage(preview,pFrame);  // 显示预览
       
		//// 图像写入文件
		//cvGrabFrame(pCapture); 
		//img = cvRetrieveFrame(pCapture); 
		cvWriteFrame(writer,pFrame); // 添加图像到视频文件
        
		char c=cvWaitKey(10);	// 延时33 毫秒

        if(c==27)break;  // ESC 退出
    }  
        cvReleaseImage(&pFrame);  
        cvReleaseVideoWriter(&writer);    
        cvDestroyWindow("video");
}

/**
 * 标记帧信息
 * 在每帧上标记眨眼参数，最好能绘制折线图
 */
void CVideoproc::MarkVideo( const char *videoname )
{ 
    CvCapture *capture = cvCaptureFromAVI(videoname);  // 从文件创建捕捉器
    int count_tmp = 0;//计数总帧数   

	CvFont font; 
    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 1, 1, 1, 2, 8);     
    
	zInitialize("阳光人脸识别二次开发包：仅授予[武汉晨扬电子科技有限公司]二次开发权利");
	DLL_OUT_FACE_STRUCT face_struct;
	LONG tid = zCreateOneThreadObject(0);
	zSetA( tid, de_is_second_locate_eye_infection_CALC_EYEWHITE, 1 );

	IplImage *frame = NULL; // 视频帧指针缓存
	int i= 0;
	char tmpfile[32]={'\0'}; 
	char tmpClose[128] = {0};
	while((frame = cvQueryFrame(capture)) != NULL)  
    {  
		face_struct.eye1_x = 0;
		zFaceLocate_BmpData(	tid, (BYTE*)frame->imageData, frame->width, frame->height, 24, 
								1,  50, &face_struct );
	

		if( 0 != face_struct.eye1_x )
		{
			memset(tmpClose, 0, sizeof tmpClose);
			sprintf(tmpClose,"closeeys:%ld",face_struct.CloseEyeBelievable);//使用帧号作为图片名
			cvPutText(frame,tmpClose, cvPoint(50,50), &font, CV_RGB(255,0,0));
			
		}

		sprintf(tmpfile,"%d.jpg",i);//使用帧号作为图片名
		cvSaveImage(tmpfile,frame);
		i++;
    } 

    cvReleaseCapture(&capture);

	//// 反初始化
	zUnInitialize();
    return ;  
}
