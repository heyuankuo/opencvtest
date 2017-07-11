#include <iostream>
#include <fstream>
#include <windows.h>
#include "opencvref.h"

using std::ifstream;
using std::ios_base;
using namespace cv;
using std::stringstream;

void trace( char *s)
{
	std::cout << s << std::endl;
}

class FrameProcessor 
{
public:
	virtual void process(Mat &input, Mat &ouput) = 0;
};

class BGFGSegmentor : public FrameProcessor 
{
	Mat gray;//��ǰ֡�Ҷ�ͼ
	Mat background;//����ͼ����ʽΪ32λ����
	Mat backImage;//CV_8U��ʽ����ͼ
	Mat foreground;//ǰ��ͼ
	double learningRate;//ѧϰ��
	int threshold;//��ֵ����ȥ�Ŷ�
public:
	BGFGSegmentor() :threshold(30), learningRate(0.6) {}

	//֡������
	void process(Mat &frame, Mat &output) 
	{
		cvtColor(frame, gray, CV_BGR2GRAY); // ת��Ϊ�Ҷ�ͼ

		if (background.empty())			
			gray.convertTo(background, CV_32F);// ��һ֡

		//����תΪCV_8U��ʽ�Ա���ȡ�͵�ǰ֡��ľ���ֵ
		background.convertTo(backImage, CV_8U); 
		
		absdiff(backImage, gray, foreground); // ��ǰ֡�뱳���Ĳ��		
		cv::threshold(foreground, output, threshold, 255, THRESH_BINARY_INV); // ���˵�ǰ�����뱳����𲻴���Ŷ���
		
		accumulateWeighted(gray, background, learningRate, output); // ���±�����output��Ϊ����
	}
};

class VideoProcessor {
private:
	VideoCapture	caputure;	
	VideoWriter		writer; //д��Ƶ������	
	string			Outputfile;//����ļ���

	int currentIndex;
	int digits;
	string extension;
	FrameProcessor *frameprocessor;
	
	void(*process)(Mat &, Mat &); //ͼ������ָ��
	bool callIt;
	string WindowNameInput;
	string WindowNameOutput;
	
	int delay;			//��ʱ
	long fnumber;
	
	long frameToStop;		//��frameToStopֹͣ	
	bool stop;				//��ͣ��־	
	vector<string> images;	//ͼ��������Ϊ������Ƶ��


	//������
public:
	VideoProcessor() : callIt(true), delay(0), fnumber(0), stop(false), digits(0), frameToStop(-1) {}
	//����ͼ������
	void setFrameProcessor(void(*process)(Mat &, Mat &)) {
		frameprocessor = 0;
		this->process = process;
		CallProcess();
	}
	//����Ƶ
	bool setInput(string filename) {
		fnumber = 0;
		//���Ѵ򿪣��ͷ����´�
		caputure.release();
		return caputure.open(filename);
	}
	//����������Ƶ���Ŵ���
	void displayInput(string wn) {
		WindowNameInput = wn;
		namedWindow(WindowNameInput);
	}
	//���������Ƶ���Ŵ���
	void displayOutput(string wn) {
		WindowNameOutput = wn;
		namedWindow(WindowNameOutput);
	}
	//���ٴ���
	void dontDisplay() {
		destroyWindow(WindowNameInput);
		destroyWindow(WindowNameOutput);
		WindowNameInput.clear();
		WindowNameOutput.clear();
	}

	//����
	void run() {
		Mat frame;
		Mat output;
		if (!isOpened())
			return;
		stop = false;
		while (!isStopped()) {
			//��ȡ��һ֡
			if (!readNextFrame(frame))
				break;
			if (WindowNameInput.length() != 0)
				imshow(WindowNameInput, frame);
			//�����֡
			if (callIt) {
				if (process)
					process(frame, output);
				else if (frameprocessor)
					frameprocessor->process(frame, output);
			}
			else {
				output = frame;
			}
			if (Outputfile.length()) {
				cvtColor(output, output, CV_GRAY2BGR);
				writeNextFrame(output);
			}
			if (WindowNameOutput.length() != 0)
				imshow(WindowNameOutput, output);
			//������ͣ��������������
			if (delay >= 0 && waitKey(delay) >= 0)
				waitKey(0);
			//����ָ����ͣ�����˳�
			if (frameToStop >= 0 && getFrameNumber() == frameToStop)
				stopIt();
		}
	}
	//��ͣ����λ
	void stopIt() {
		stop = true;
	}
	//��ѯ��ͣ��־λ
	bool isStopped() {
		return stop;
	}
	//������Ƶ�򿪱�־
	bool isOpened() {
		return  caputure.isOpened() || !images.empty();
	}
	//������ʱ
	void setDelay(int d) {
		delay = d;
	}
	//��ȡ��һ֡
	bool readNextFrame(Mat &frame) {
		if (images.size() == 0)
			return caputure.read(frame);
		else {
			if (itImg != images.end()) {
				frame = imread(*itImg);
				itImg++;
				return frame.data ? 1 : 0;
			}
			else
				return false;
		}
	}

	void CallProcess() {
		callIt = true;
	}
	void  dontCallProcess() {
		callIt = false;
	}
	//����ֹͣ֡
	void stopAtFrameNo(long frame) {
		frameToStop = frame;
	}
	// ��õ�ǰ֡��λ��
	long getFrameNumber() {
		long fnumber = static_cast<long>(caputure.get((CV_CAP_PROP_POS_FRAMES)));
		return fnumber;
	}

	//���֡��С
	Size getFrameSize() {
		if (images.size() == 0) {
			// ����Ƶ�����֡��С
			int w = static_cast<int>(caputure.get(CV_CAP_PROP_FRAME_WIDTH));
			int h = static_cast<int>(caputure.get(CV_CAP_PROP_FRAME_HEIGHT));
			return Size(w, h);
		}
		else {
			//��ͼ����֡��С
			cv::Mat tmp = cv::imread(images[0]);
			return (tmp.data) ? (tmp.size()) : (Size(0, 0));
		}
	}

	//��ȡ֡��
	double getFrameRate() {
		return caputure.get(CV_CAP_PROP_FPS);
	}
	vector<string>::const_iterator itImg;
	bool setInput(const vector<string> &imgs) {
		fnumber = 0;
		caputure.release();
		images = imgs;
		itImg = images.begin();
		return true;
	}

	void  setFrameProcessor(FrameProcessor *frameprocessor) {
		process = 0;
		this->frameprocessor = frameprocessor;
		CallProcess();
	}

	//��ñ�������
	int getCodec(char codec[4]) {
		if (images.size() != 0)
			return -1;
		union { // ���ݽṹ4-char
			int value;
			char code[4];
		} returned;
		//��ñ���ֵ
		returned.value = static_cast<int>(
			caputure.get(CV_CAP_PROP_FOURCC));
		// get the 4 characters
		codec[0] = returned.code[0];
		codec[1] = returned.code[1];
		codec[2] = returned.code[2];
		codec[3] = returned.code[3];
		return returned.value;
	}


	bool setOutput(const string &filename, int codec = 0, double framerate = 0.0, bool isColor = true) {
		//�����ļ���
		Outputfile = filename;
		//�����չ��
		extension.clear();
		//����֡��
		if (framerate == 0.0) {
			framerate = getFrameRate();
		}
		//��ȡ����ԭ��Ƶ�ı��뷽ʽ
		char c[4];
		if (codec == 0) {
			codec = getCodec(c);
		}
		return writer.open(Outputfile,
			codec,
			framerate,
			getFrameSize(),
			isColor);
	}

	//�����Ƶ֡��ͼƬfileme+currentIndex.ext,��filename001.jpg
	bool setOutput(const string &filename,//·��
		const string &ext,//��չ��
		int numberOfDigits = 3,//����λ��
		int startIndex = 0) {//��ʼ����
		if (numberOfDigits < 0)
			return false;
		Outputfile = filename;
		extension = ext;
		digits = numberOfDigits;
		currentIndex = startIndex;
		return true;
	}

	//д��һ֡
	void writeNextFrame(Mat &frame) 
	{
		//�����չ����Ϊ�գ�д��ͼƬ�ļ���
		if (extension.length()) 
		{
			stringstream ss;
			// ss << Outputfile << setfill('0') << setw(digits) << currentIndex++ << extension;
			imwrite(ss.str(), frame);
		}
		//��֮��д����Ƶ�ļ���
		else {
			writer.write(frame);
		}
	}

};


int main( int argc, char *argv[])
{
	VideoProcessor processor;
	//  FeatureTracker tracker;  
	BGFGSegmentor tracker;
	//��������Ƶ  
	processor.setInput("bike.avi");
	processor.displayInput("Current Frame");
	processor.displayOutput("Output Frame");
	//����ÿһ֡����ʱ  
	processor.setDelay(1000. / processor.getFrameRate());
	//����֡����������������  
	processor.setFrameProcessor(&tracker);
	//   processor.setOutput ("./bikeout.avi");  
	//    processor.setOutput ("bikeout",".jpg");  
	processor.run();

	system("pause");
	return 0;
}