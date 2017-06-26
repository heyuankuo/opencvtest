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
 * �˺�������¼�� �� Ԥ����Ƶ
 * ע���˺���Ϊ��������
 * @param			videoname		[in]		�������Ƶ����
 * @param			preview			[in]		Ԥ���������
 */
void CVideoproc::RecordVideo( const char *videoname,
								const char *preview)
{ 

	//��ȡĬ������ͷ���˴�Ӧ�м��
	CvCapture* pCapture = cvCreateCameraCapture(0); 

    //����������Ԥ�����ڶ��� 
    cvNamedWindow( preview, 1); 

	// ������Ƶ����
    int isColor = 1;  
    int fps = 30; 
    int frameW = 640; 
    int frameH = 480;  
	CvVideoWriter *writer = cvCreateVideoWriter(videoname,CV_FOURCC('X','V','I','D'),fps,cvSize(frameW,frameH),isColor);  
  
	IplImage* pFrame = NULL;  // ��Ƶ֡ ָ�뻺��
	IplImage* img   = NULL;	// ͼ������ָ�뻺��
    while(1)  
    {  
        pFrame=cvQueryFrame( pCapture );  // ����ƵԴ����һ֡ͼ��
        if(!pFrame)break; 

        cvShowImage(preview,pFrame);  // ��ʾԤ��
       
		//// ͼ��д���ļ�
		//cvGrabFrame(pCapture); 
		//img = cvRetrieveFrame(pCapture); 
		cvWriteFrame(writer,pFrame); // ���ͼ����Ƶ�ļ�
        
		char c=cvWaitKey(10);	// ��ʱ33 ����

        if(c==27)break;  // ESC �˳�
    }  
        cvReleaseImage(&pFrame);  
        cvReleaseVideoWriter(&writer);    
        cvDestroyWindow("video");
}

/**
 * ���֡��Ϣ
 * ��ÿ֡�ϱ��գ�۲���������ܻ�������ͼ
 */
void CVideoproc::MarkVideo( const char *videoname )
{ 
    CvCapture *capture = cvCaptureFromAVI(videoname);  // ���ļ�������׽��
    int count_tmp = 0;//������֡��   

	CvFont font; 
    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 1, 1, 1, 2, 8);     
    
	zInitialize("��������ʶ����ο�������������[�人������ӿƼ����޹�˾]���ο���Ȩ��");
	DLL_OUT_FACE_STRUCT face_struct;
	LONG tid = zCreateOneThreadObject(0);
	zSetA( tid, de_is_second_locate_eye_infection_CALC_EYEWHITE, 1 );

	IplImage *frame = NULL; // ��Ƶָ֡�뻺��
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
			sprintf(tmpClose,"closeeys:%ld",face_struct.CloseEyeBelievable);//ʹ��֡����ΪͼƬ��
			cvPutText(frame,tmpClose, cvPoint(50,50), &font, CV_RGB(255,0,0));
			
		}

		sprintf(tmpfile,"%d.jpg",i);//ʹ��֡����ΪͼƬ��
		cvSaveImage(tmpfile,frame);
		i++;
    } 

    cvReleaseCapture(&capture);

	//// ����ʼ��
	zUnInitialize();
    return ;  
}
