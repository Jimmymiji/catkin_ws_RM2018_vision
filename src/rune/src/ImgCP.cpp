#include "RMVideoCapture.hpp"
#include "ImgCP.hpp"
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <iostream>  
#include <cmath>
#include "findRect.hpp"
#include "MnistRecognizer.h"
#include "DigitRecognizer.h"
#include "Settings.h"
#include "angleSol.h"
#include "ros/ros.h"
#include "geometry_msgs/Point.h"
using namespace std;
using namespace cv;
volatile unsigned int pIdx = 0;
volatile unsigned int cIdx = 0;
int previousDigits[5] = {-3,-3,-3,-3,-3};
volatile int previousIndex = -1;
volatile bool failure = false;
struct ImageData {
	Mat img;
	unsigned int frame;
};
ImageData data[BUFFER_SIZE];
void DigitThread(Mat img,int& ans,bool failure)
{
    //cout<<"thread 3 running"<<endl;
    DigitRecognizer dt;
    dt.preprocessRGB(img,img);
    if(!dt.findDigits(img))
    {
        //cout<<"thread 3 end 1"<<endl;
        ans = -1;
        return;
    }
    if(!dt.getAns())
    {
       // cout<<"thread 3 end 2"<<endl;
        ans = -1;
        return;
    }
    bool repeat = true;
    for(int i = 0; i<5;i++)
    {
        if(previousDigits[i] != dt.answers[i] )
        {
            repeat = false;
        }
        previousDigits[i] = dt.answers[i];
        cout<< "  "<< previousDigits[i] ;
    }
    cout<<endl;
    if(!repeat && !failure)
    {
        previousIndex = 0;
    }
    else
    {
        previousIndex++;
    }
    ans = dt.answers[previousIndex];
    cout<<"thread 3 end 3, we shoot number "<< ans <<endl;
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
        //const char* cp =  cameraPath + cameraNumber;
        RMVideoCapture cap("/dev/video1", 3); 
		cap.setVideoFormat(640, 480, 1);
		//cap.setExposureTime(0, settings->cameraSetting.ExposureTime);//settings->exposure_time);
		cap.startStream();
		cap.info();
		while(1)
		{
            cout<<"image producer running"<<endl;
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
    ros::init( argc, argv,"rune");
    ros::NodeHandle n;
    ros::Publisher rune_pub = n.advertise<geometry_msgs::Point>("rune_locations",5);
    clock_t start, end;
    geometry_msgs::Point target;
    cout<<"ros publisher initialized"<<endl;
    int prevoiusSudokus[9];
    int hitIndex = 0;
    waitKey(100);
    start = clock();
    cout<<"start loop"<<endl;
    while(true)
    {
        cout<<"*"<<endl;
        end = clock();
        cout<<"hz "<<1/((double)(end-start)/CLOCKS_PER_SEC)<<endl;
        while (pIdx - cIdx <= 0)
        cout<<"no image input, pidx: "<< pIdx << "  cIdx: "<< cIdx<<endl;
        start = clock();
        Mat img,img1;
		data[cIdx % BUFFER_SIZE].img.copyTo(img);
        data[cIdx % BUFFER_SIZE].img.copyTo(img1);
        //imshow("input",img);
		unsigned int frameNum = data[cIdx % BUFFER_SIZE].frame;
		++cIdx;
        int hitIndex ;
        imshow("original",img1);
        waitKey(10);
        thread t3(DigitThread,img1,std::ref(hitIndex),failure);
        Mat image;
        cvtColor(img, image, CV_BGR2GRAY);
        Mat binary;
        threshold(image,binary,150,255,CV_THRESH_BINARY);
        morphologyEx( binary,  binary, MORPH_OPEN, getStructuringElement(MORPH_RECT,Size(3,3)));
        vector<vector<Point> > squares;
        findSquaresBinary(binary,squares);
        vector<RotatedRect> rects;
        t3.join();
        if(hitIndex == -1)
        {
            waitKey(5);
            failure = true;
            continue;
        }
        if(!checkRects(binary,squares,rects))
        {
            //t3.join();
            target.x = -1;
	        target.y = -1;
	        target.z = -1;
	        rune_pub.publish(target);
	        ROS_INFO("x: %f y: %f z: %f",target.x,target.y,target.z);
            cout<<"not enough mnists"<<endl;
            failure = true;
            continue;
        }
        bool outOfImg = false;
        MnistRecognizer MR;
        for(int i = 0; i<9;i++)
        {
            cout<<"!"<<endl;
            Rect t = rects[i].boundingRect();
            if(!(0 <= t.x && 0 <= t.width && t.x + t.width <= img.cols && 0 <= t.y && 0 <= t.height && t.y + t.height <= img.rows))
            {
                outOfImg = true;
                break;
            }
            MR.mnistImgs.push_back(img(rects[i].boundingRect()));
            }
            if(outOfImg)
            {
               // waitKey(10);
                failure = true;
                continue;
            }
            if(MR.classify())
            {   
                for(int i = 1;i<=9;i++)
                putText(img,to_string(i),rects[MR.mnistLabels[i]].center, FONT_HERSHEY_SIMPLEX, 1 , Scalar(0,255,255),3);
                //imshow("a",img);
                Rect t = rects[hitIndex].boundingRect();
                AngleSolver ag;
                ag.setDistortionCoefficients(s);
	            ag.setCameraMAtrix(s);
	            ag.setRealWorldTargetS(s);
                vector<Point2f> input;
                input.push_back(Point2f(t.x,t.y));
                input.push_back(Point2f(t.x,t.y+t.height));
                input.push_back(Point2f(t.x+t.width,t.y));
                input.push_back(Point2f(t.x+t.width,t.y+t.height));
	            if(!ag.setImageTargetS(input,img))
	 	        {
                    cout<< "setImageTarget gg " <<endl;
                }
                else
                {
	                ag.getRotation_Translation_Matrix();
	                ag.getPositionInfo(target.x,target.y,target.z);
                    rune_pub.publish(target);
                   	ROS_INFO("x: %f y: %f z: %f",target.x,target.y,target.z);
                    waitKey(shootingDelay);
	                ag.sendAns(img);
                    failure = false;
                }
            }
            else
            {
               // waitKey(10);
                failure = true;
                continue;
            }
            
                waitKey(5);
     }
}

