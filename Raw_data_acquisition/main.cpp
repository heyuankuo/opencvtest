#include <iostream>
#include <windows.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>  
#include <signal.h> 
#include "SunLightFace.h"

#pragma comment(lib, "SunLightFace.lib")

#pragma comment(lib,  "opencv_calib3d246d.lib")
#pragma comment(lib,  "opencv_core246d.lib")
#pragma comment(lib,  "opencv_features2d246d.lib")
#pragma comment(lib,  "opencv_flann246d.lib")
#pragma comment(lib,  "opencv_gpu246d.lib")
#pragma comment(lib,  "opencv_highgui246d.lib")
#pragma comment(lib,  "opencv_imgproc246d.lib")
#pragma comment(lib,  "opencv_legacy246d.lib")
#pragma comment(lib,  "opencv_ml246d.lib")
#pragma comment(lib,  "opencv_objdetect246d.lib")
#pragma comment(lib,  "opencv_ts246d.lib")
#pragma comment(lib,  "opencv_video246d.lib")

#include <fstream>
using std::ifstream;
using std::ios_base;
using std::ofstream;


#define LEN 128  
  
IplImage *FrameImage;  
IplImage *camFrame = NULL;  
CvCapture *cam = NULL;  
cv::Mat Image;  
int GaussianBlurValue = 3;  
cv::Mat dstImage ;  

DLL_OUT_FACE_STRUCT face_struct;
  
static void on_GaussianBlur(int, void *);  
  
//-------信号函数------------  
void process(int )  
{  
   cvReleaseCapture(&cam);//释放CvCapture结构  
   exit(0);  
}  

int main(int argc, char *argv[])
{

	float a = 0.45;
	// int a = 200;
	int *b = (int *)&a;

	zInitialize("阳光人脸识别二次开发包：仅授予[武汉晨扬电子科技有限公司]二次开发权利");
	// 设置摄像头宽高
	cv::VideoCapture capture(0);  
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);  
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	// 捕捉一帧视频
	int count=0;  
  
	LONG tid = zCreateOneThreadObject(0);
	zSetA( tid, de_is_second_locate_eye_infection_CALC_EYEWHITE, 1 );

	ofstream ofs( "1.txt", ios_base::ate);

	const char *data_format = "%d,";
	char data_value[64] = {0};
    while (1) 
	{  
		cv::Mat frame;  
        capture>>frame; //载入图像  
  
        if (frame.empty()) 
		{ //判断图像是否载入  
			std::cout<<"can not load the frame"<<std::endl;  
        } 
		else 
		{  
              
            if (count == 50) 
			{  
				ofs.close();
				ofs.clear();
				
				break;  
            }  
  
            memset( &face_struct, 0, sizeof face_struct);
			zFaceLocate_BmpData(	tid, 
									frame.data,frame.cols,frame.rows, 24, 
									1,  1.0f, &face_struct );

			if( 0 != face_struct.eye1_x )
			{
				count++;
				std::cout << face_struct.CloseEyeBelievable << std::endl;
				memset(data_value, 0, sizeof data_value);
				sprintf( data_value, data_format, face_struct.CloseEyeBelievable);
				ofs.write( data_value, strlen(data_value));
			}
			// imshow("camera", frame); 
			char c=cv::waitKey(5); //延时30毫秒  
            if (c == 27) //按ESC键退出  
			{
				ofs.close();
				ofs.clear();
				break;
			}
        }  
    } 

	/*cv::Mat dst( 1, 200, CV_64FC1);
	
	IplImage *dst = cvCreateImage(cvGetSize(src), IPL_DEPTH_64F, 1); 
	cvZero(dst);
	cvFFT( src, dst, CV_DXT_FORWARD);
	for( int i = 0; i < 200; i++)
	{
		std::cout << i << ":\t\t" <<*((double *)(dst->imageData) + i) << std::endl;
	}*/
	
	
	//// 反初始化
	zUnInitialize();

	return 0;
}

//------高斯滤波轨迹条回调函数------------  
static void on_GaussianBlur(int, void *)  
{  
	GaussianBlur(Image,dstImage,cv::Size(GaussianBlurValue*  2+1,GaussianBlurValue*2+1),0,0);   
}  
