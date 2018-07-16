#include "RMVideoCapture.hpp"
#include "ImgCP.hpp"
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <iostream>  
#include <cmath>
#include <string>
#include <limits.h>
#include <unistd.h>
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
std::string getexepath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return std::string( result, (count > 0) ? count : 0 );
}
void preprocessRGB(Mat img, Mat& result)
{
    vector<Mat> channels;
    split(img,channels); 
    Mat Red = channels.at(2);
    Mat Blue = channels.at(0);
    Mat Green = channels.at(1);
    Mat R_B = Red - Blue;
    Mat G_B = Green - Blue;
    result = R_B & G_B;
    imshow("result",result);
    waitKey(1);
    return ;
}
bool DigitThread(Mat img,vector<int>& ans,Settings & s)
{
    //cout<<"thread 3 running"<<endl;
    imshow("digits",img);
    waitKey(1);
    DigitRecognizer dt(s);
   // dt.preprocessRGB(img,img);
   img.copyTo(dt.binary);
    if(!dt.findDigits())
    {
        //cout<<"thread 3 end 1"<<endl
        cout<<" no digit found "<<endl;
        return false;
    }
    if(dt.getAns())
    {
       // cout<<"thread 3 end 2"<<endl;
        for(int i = 0; i<5;i++)
        {
            cout<< "  "<< dt.ans[i];
            ans.push_back(dt.ans[i]);
        }
        //dt.recordResults(cIdx);
        return true;
    }
    
    //dt.recordResults(cIdx);
    return false;
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
        RMVideoCapture cap("/dev/v4l/by-id/usb-HD_Camera_Manufacturer_Stereo_Vision_1_Stereo_Vision_1-video-index0"); 
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
    Settings s("bigSetting.xml","2.yml");
    if(!s.load())
    {
	    cout<<"where is my setting file?"<<endl;  
        cout<<"current path"<<  getexepath()<<endl;    
	    return ;
    }
    cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());

	if(!loadData(kNearest))
	{
		cout << "loadData NOT done" << endl;
        return ;
	}
	cout << "loadData done" << endl;
    ros::init(argc, argv, "runeBig");
    ros::NodeHandle n;
    ros::Publisher rune_pub =
    n.advertise<geometry_msgs::Point>("big_rune_locations", 1);
    geometry_msgs::Point target;
    cout << "ros publisher initialized" << endl;
    Master mst;
    clock_t start,end;
    start = clock();
    ofstream myfile;
    myfile.open("record.txt");
    while(true)
    {
        start = clock();
        while (pIdx - cIdx == 0);
        Mat img,result, img1;
		data[cIdx % BUFFER_SIZE].img.copyTo(img);
        data[cIdx % BUFFER_SIZE].img.copyTo(img1);
		unsigned int frameNum = data[cIdx % BUFFER_SIZE].frame;
		++cIdx;
        cout<<"p1"<<endl;
        preprocessRGB(img,result);
        cout<<"p2"<<endl;
        Mat binary;
        double Mean = mean(result)[0];
        threshold(result,binary,Mean*s.imgCPSetting.mean,255,CV_THRESH_BINARY);
        // imshow("binary1",binary);
        // waitKey(1);
        int dilateSize1 = s.imgCPSetting.dilateSize1;
        morphologyEx( binary,binary, MORPH_DILATE, getStructuringElement(MORPH_RECT,Size(dilateSize1,dilateSize1)));
        // imshow("binary2",binary);
        // waitKey(1);
        vector<vector<Point>> squares;
        cout<<"________1________"<<endl;
        findRects(binary,squares,s);
        drawSquares(img1,squares);
        vector<RotatedRect> rects;
        cout<<"_________2_____________"<<endl;
        if(!checkRects(img,squares,rects,s))
        {
            cout<<"_______________3______________"<<endl;
            target.x = -1;
            target.y = -1;
            target.z = -1;
            cout << "not enough mnists" << endl;
            mst.Fail();
           // myfile<<to_string(cIdx)<<": 1"<<endl;
            continue;
        }
        cout<<"___________________4___________"<<endl;
        FNRecognizer haha;
        bool outOfImg = false;
        for (int i = 0; i < 9; i++) 
        {
            Rect t = rects[i].boundingRect();
            if (!(0 <= t.x && 0 <= t.width && t.x + t.width <= img.cols && 0 <= t.y &&
            0 <= t.height && t.y + t.height <= img.rows)) 
            {
                outOfImg = true;
                break;
            }
        }
        if(outOfImg)
        {
            mst.Fail();
             //myfile<<to_string(cIdx)<<": 2"<<endl;
            continue;
        }
        cout<<"____________5__________"<<endl;
        for(int i = 0;i<9;i++)
        {
           cout<<"_____________77_______"<<endl;
           int result = haha.predict(kNearest,img(rects[i].boundingRect()));
           cout<<"_____________88_______________"<<endl;
           haha.relations[result] = i;
           //putText(img,to_string(result),rects[i].center,FONT_HERSHEY_SIMPLEX,1,Scalar(255,0,0),3);
         }
        // imshow("haha",img);
        // waitKey(1);
        if(haha.relations.size()!=9)
        {
            mst.Fail();
             //myfile<<to_string(cIdx)<<": 2"<<endl;
            continue;
        }
        for (int i = 1; i <= 9; i++) 
        {
            mst.currentMNIST.push_back(haha.relations[i]);
        }
        int DigitLeft = rects[0].center.x;
        int DigitRight = rects[2].center.x;
        int DigitDown  = (rects[0].boundingRect().y + rects[2].boundingRect().y)/2;
        int DigitUp = DigitDown - (rects[0].boundingRect().height + rects[2].boundingRect().height);
        if(DigitUp<0)
        DigitUp = 0;
        Mat ROIOfDigits = binary(Range(DigitUp,DigitDown),Range(DigitLeft,DigitRight));
        //imwrite(to_string(cIdx)+"d.png",ROIOfDigits);
        cout<<"________________6___________________"<<endl;
        if(!DigitThread(ROIOfDigits, mst.currentDigits,s))
        {
            mst.Fail();
            continue;
        }
        cout<<"___________________7________________"<<endl;
        string anss = to_string(mst.currentDigits[0]) +to_string(mst.currentDigits[1]) +to_string(mst.currentDigits[2]) +to_string(mst.currentDigits[3]) + to_string(mst.currentDigits[4]);
        putText(img,anss,Point(100,200),FONT_HERSHEY_SIMPLEX,1,Scalar(255,0,0),3);
        imshow("digits",img);
        myfile<<to_string(cIdx)<<" "<<anss<<endl;
        int hitNumber = mst.whichToShootSemiAuto(myfile,s.imgCPSetting.hitNumber);
        if(hitNumber == -1)
        {
            mst.Fail();
             //myfile<<to_string(cIdx)<<": 3"<<endl;
            continue;
        }
        int hitIndex = haha.relations[hitNumber];
        AngleSolver ag(s);
        sort(rects.begin(),rects.end(),ascendingY);                
        sort(rects.begin(),rects.begin()+3,ascendingX);          
        sort(rects.begin()+3,rects.begin()+6,ascendingX);
        sort(rects.begin()+6,rects.begin()+9,ascendingX);
        for(int i = 0; i<9;i++)
        {
            ag.centers.push_back(rects[i].center);
        }
        ag.setRealWorldTargetB(s,hitIndex);
        ag.setImageTargetB(img,hitIndex);
        ag.getPositionInfo(target.x,target.y,target.z);
        rune_pub.publish(target);
        ROS_INFO("x: %f y: %f z: %f", target.x, target.y, target.z);
        ag.sendAns(img);
        //imwrite(to_string(cIdx)+".png",img);
        mst.Fail();
        //myfile<<to_string(cIdx)<<": ********************"<<endl;
    
    }
}

