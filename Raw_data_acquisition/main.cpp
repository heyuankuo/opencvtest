#include <iostream>
#include <windows.h>
#include <fstream>

#include <signal.h> 

#include "Videoproc.h"

using std::ifstream;
using std::ios_base;
using std::ofstream;

#define LEN 128  

int GaussianBlurValue = 3;  
  
static void on_GaussianBlur(int, void *);  
  
//-------�źź���------------  
//void process(int )  
//{  
//   cvReleaseCapture(&cam);//�ͷ�CvCapture�ṹ  
//   exit(0);  
//} 



int main(int argc, char *argv[])
{
	CVideoproc vp;
	// vp.RecordVideo( "hyk.avi", "hyk");
	vp.MarkVideo( "VID_20170626_173016.mp4");

	return 0;
}

//------��˹�˲��켣���ص�����------------  
//static void on_GaussianBlur(int, void *)  
//{  
//	GaussianBlur(Image,dstImage,cv::Size(GaussianBlurValue*  2+1,GaussianBlurValue*2+1),0,0);   
//}  
