#include "RMVideoCapture.hpp"
#include "ImgCP.hpp"
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <iostream>  
#include <cmath>
#include "findRect.hpp"
#include "FNRecognizer.h"
#include "DigitRecognizer.h"
#include "Settings.h"
#include "angleSol.h"
#include "master.h"
#include "LRBlock.h"
#include "ros/ros.h"
#include "geometry_msgs/Point.h"
using namespace std;
using namespace cv;
volatile unsigned int pIdx = 0;
volatile unsigned int cIdx = 0;
volatile bool failure = false;
struct ImageData {
	Mat img;
	unsigned int frame;
};
ImageData data[BUFFER_SIZE];
void preprocessRGB(Mat img, Mat& result)
{
    vector<Mat> channels;
    split(img,channels); 
    Mat Red = channels.at(2);
    Mat Blue = channels.at(0);
    Mat Green = channels.at(1);
    Mat R_B = Red - Blue;
    Mat G_B = Green - Blue;
    imshow("R-B",R_B);
    waitKey(1);
    imshow("G_B",G_B);
    waitKey(1);
    result = R_B & G_B;
    imshow("result",result);
    // result = R_B & R_G;
    // threshold(result,result,90,255,THRESH_BINARY);
    waitKey(1);
    return ;
}
void DigitThread(Mat img,vector<int>& ans)
{
    //cout<<"thread 3 running"<<endl;
    DigitRecognizer dt;
    dt.preprocessRGB(img,img);
    if(!dt.findDigits())
    {
        //cout<<"thread 3 end 1"<<endl
        cout<<" no digit found "<<endl;
        return;
    }
    if(dt.getAns())
    {
       // cout<<"thread 3 end 2"<<endl;
        for(int i = 0; i<5;i++)
        {
            cout<< "  "<< dt.ans[i];
            ans.push_back(ans[i]);
        }
        //dt.recordResults(cIdx);
        return;
    }
    
    //dt.recordResults(cIdx);
    return;
}
void ImgCP::ImageProducer()
{
	if (isVideoMode)
	{
		if (videoPath == NULL)
		{
			cout << "excuse me?" << endl;
			return;
		}
		VideoCapture cap(videoPath);
		if (!cap.isOpened())
		{
			cout << "not open" << endl;
			return;
		}
		cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
		while(1)
		{
			while (pIdx - cIdx >= BUFFER_SIZE);
			Mat temp;
			cap >> temp; 
			resize(temp, temp, Size(640, 480), 0, 0, INTER_CUBIC);
			temp.copyTo(data[pIdx % BUFFER_SIZE].img);
			data[pIdx % BUFFER_SIZE].frame++;
			++pIdx;
		}
	}
	else
	{
		std::string cameraPath = "/dev/video";
        RMVideoCapture cap("/dev/video1", 3); 
		cap.setVideoFormat(640, 480, 1);
		cap.startStream();
		cap.info();
		while(1)
		{
        	while (pIdx - cIdx >= BUFFER_SIZE);
			cap >> data[pIdx % BUFFER_SIZE].img;
			data[pIdx % BUFFER_SIZE].frame = cap.getFrameCount();
			++pIdx;
		}
	}
}

void ImgCP::ImageConsumer(int argc, char** argv)
{
    cout<<"start"<<endl;
    while(pIdx == 0);
    Settings s("setting.xml","1.yml");
    if(!s.load())
    {
	    cout<<"where is my setting file?"<<endl;        
	    return ;
    }
    cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());

	if(!loadData(kNearest))
	{
		cout << "loadData NOT done" << endl;
        return ;
	}
	cout << "loadData done" << endl;
    clock_t start,end;
    start = clock();
    while(true)
    {
        start = clock();
        while (pIdx - cIdx == 0);
        Mat img,result;
		data[cIdx % BUFFER_SIZE].img.copyTo(img);
		unsigned int frameNum = data[cIdx % BUFFER_SIZE].frame;
		++cIdx;
        preprocessRGB(img,result);
        Mat binary;
        double Mean = mean(result)[0];
        threshold(result,binary,Mean*15,255,CV_THRESH_BINARY);
        imshow("binary1",binary);
        waitKey(1);
        morphologyEx( binary,binary, MORPH_DILATE, getStructuringElement(MORPH_RECT,Size(7,7)));
        imshow("binary2",binary);
        waitKey(1);
        vector<vector<Point>> squares;
        findRects(binary,squares);
        //drawSquares(img,squares);
        vector<RotatedRect> rects;
        if(!checkRects(img,squares,rects))
        {
            continue;
        }
        FNRecognizer haha;
        imshow("wtf",img);
        waitKey(1);
        for(int i = 0;i<9;i++)
        {
           int result = haha.predict(kNearest,img(rects[i].boundingRect()));
           putText(img,to_string(result),rects[i].center,FONT_HERSHEY_SIMPLEX,1,Scalar(255,0,0),3);
        }
        imshow("haha",img);
        waitKey(1);
        end = clock();
        cout<<"hz: "<<1/((double)(end-start)/CLOCKS_PER_SEC)<<endl;
        
    }
}

