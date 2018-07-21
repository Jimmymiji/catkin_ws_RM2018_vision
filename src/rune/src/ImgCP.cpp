#include "DigitRecognizer.h"
#include "ImgCP.hpp"
#include "LRBlock.h"
#include "MnistRecognizer.h"
#include "RMVideoCapture.hpp"
#include "Settings.h"
#include "angleSol.h"
#include "findRect.hpp"
#include "geometry_msgs/Point.h"
#include "master.h"
#include "ros/ros.h"
#include <cmath>
#include <iostream>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdexcept>
#include <stdio.h>
#include <string>
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

std::string exec(const char *cmd) {
  char buffer[128];
  std::string result = "";
  FILE *pipe = popen(cmd, "r");
  if (!pipe)
    throw std::runtime_error("popen() failed!");
  try {
    while (!feof(pipe)) {
      if (fgets(buffer, 128, pipe) != NULL)
        result += buffer;
    }
  } catch (...) {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return result;
}

bool DigitThread(Mat img, vector<int> &answer,Settings& s) {
  DigitRecognizer dt(s);
  dt.preprocessRGB(img, img);
  answer.clear();
  if (!dt.findDigits()) {
    cout << " no digit found " << endl;
    return false;
  }
  string haha ;
  if (dt.getAns()) {
    //cout<<"------2--------"<<endl;
    for (int i = 0; i < 5; i++) {
      answer.push_back(dt.ans[i]);
      haha = haha+" "+to_string(dt.ans[i]);
      cout<<dt.ans[i]<<" ";
    }
    cout<<endl;
    // dt.recordResults(cIdx);
    return true;
  }

  // dt.recordResults(cIdx);
  return false;
}
void ImgCP::ImageProducer() {
  if (isVideoMode) {
    if (videoPath == NULL) {
      cout << "no video found" << endl;
      return;
    }
    VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
      cout << "video not open" << endl;
      return;
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    while (1) {
      while (pIdx - cIdx >= BUFFER_SIZE)
        ;
      Mat temp;
      cap >> temp;
      resize(temp, temp, Size(1280,720), 0, 0, INTER_CUBIC);
      temp.copyTo(data[pIdx % BUFFER_SIZE].img);
      data[pIdx % BUFFER_SIZE].frame++;
      ++pIdx;
    }
  } else {
    std::string usb_video = "/dev/video";
    std::string result =
        exec("bash /etc/lsusb.sh");
    std::cout << result;
    size_t found = result.find(usb_video);
    if (found != std::string::npos) {
      std::cout << "usb video: " << result[found + usb_video.length()]
                << std::endl;
      usb_video.push_back(result[found + usb_video.length()]);
    }
    // const char* cp =  cameraPath + cameraNumber;
    RMVideoCapture cap("/dev/v4l/by-id/usb-HD_Camera_Manufacturer_Stereo_Vision_1_Stereo_Vision_1-video-index0", 3);
    cap.setExposureTime(false,50);
    cap.setVideoFormat(1280, 720, 1);
    cap.startStream();
    cap.info();
    while (1) {
      // cout<<"image producer running"<<endl;
      while (pIdx - cIdx >= BUFFER_SIZE)
        ;
      cap >> data[pIdx % BUFFER_SIZE].img;
      data[pIdx % BUFFER_SIZE].frame = cap.getFrameCount();
      ++pIdx;
    }
  }
}

void ImgCP::ImageConsumer(int argc, char **argv) {
  cout << "start" << endl;
  while (pIdx == 0)
    ;
  Settings s("smallSetting.xml", "green.yml");
  if (!s.load()) {
    cout << "where is my setting file?" << endl;
    return;
  }
  ros::init(argc, argv, "rune");
  ros::NodeHandle n;
  ros::Publisher rune_pub =
      n.advertise<geometry_msgs::Point>("rune_locations", 1);
  clock_t start, end;
  geometry_msgs::Point target;
  cout << "ros publisher initialized" << endl;
  waitKey(100);
  cout << "start loop" << endl;
  Master mst;
   ofstream myfile,myfile1;
   myfile.open("record.txt");
   myfile1.open("record1.txt");
  LRBlock lrb;
  while (true) {
    // if(cIdx>1500)
    // {
    //     break;
    // }
    end = clock();
    cout << "hz " << 1 / ((double)(end - start) / CLOCKS_PER_SEC) << endl;
    start = clock();
    while (pIdx - cIdx == 0);
    clock_t end0 = clock();
    Mat img, img1;
    data[cIdx % BUFFER_SIZE].img.copyTo(img);
    data[cIdx % BUFFER_SIZE].img.copyTo(img1);
    // transpose(img,img);
    // transpose(img1,img1);
    // flip(img,img,0);
    // flip(img1,img1,0);
    //img1.convertTo(img1, -1, 1, -100);
    //imshow("dark",img1);
    unsigned int frameNum = data[cIdx % BUFFER_SIZE].frame;
    ++cIdx;
    // myfile<<"***********"<<to_string(cIdx)<<"********"<<endl;
    imshow("original",img);
    waitKey(1);
    mst.blueCount = lrb.countBlueBlock(img);

    // thread t3(DigitThread,img1,mst.currentDigits,s);
    Mat image;
    cvtColor(img, image, CV_BGR2GRAY);
    Mat binary;
    double Mean = mean(image)[0];
    threshold(image, binary, Mean * s.imgCPSetting.mean, 255, CV_THRESH_BINARY);
    clock_t m1 = clock();
    int erodeSize1 = s.imgCPSetting.erodeSize1;
    int erodeSize2 = s.imgCPSetting.erodeSize2;
    morphologyEx(binary, binary, MORPH_ERODE,
                 getStructuringElement(MORPH_RECT, Size(erodeSize1, erodeSize1)));
    morphologyEx(binary, binary, MORPH_ERODE,
                 getStructuringElement(MORPH_RECT, Size(erodeSize2, erodeSize2)));
    vector<vector<Point>> squares;
   imshow("binary",binary);
    findRects(binary, squares,s);
    // drawSquares(img1, squares);
    // waitKey(1);
    vector<RotatedRect> rects;
    clock_t end1 = clock();
    if (!checkRects(binary, squares, rects,s)) {
      // t3.join();
      target.x = -1;
      target.y = -1;
      target.z = -1;
      cout << "not enough mnists" << endl;
      mst.Fail();
      continue;
    }

    bool outOfImg = false;
    MnistRecognizer MR;

    for (int i = 0; i < 9; i++) {
      Rect t = rects[i].boundingRect();
      if (!(0 <= t.x && 0 < t.width && t.x + t.width <= img.cols && 0 <= t.y &&
            0 < t.height && t.y + t.height <= img.rows)) {
        outOfImg = true;
        break;
      }
      MR.mnistImgs.push_back(img(rects[i].boundingRect()));
    }
    if (outOfImg) {
      mst.Fail();
      continue;
    }
    if (MR.classify2()) {
      for (int i = 1; i <= 9; i++)
      {
         putText(img,to_string(i),rects[MR.mnistLabels[i]].center,
         FONT_HERSHEY_SIMPLEX, 1 , Scalar(255,255,0),3);
        mst.currentMNIST.push_back(MR.mnistLabels[i]);
      }
      myfile1<<"fire : "<<to_string(cIdx)<<endl;
      for(int i = 1;i<10;i++)
      {
          myfile1<<to_string(i)<<","<<to_string(MR.mnistLabels[i])<<" | ";
      }
      myfile1<<endl;
      int DigitLeft = rects[0].center.x;
      int DigitRight = rects[2].center.x;
      int DigitDown  = (rects[0].boundingRect().y + rects[2].boundingRect().y)/2;
      int DigitUp = DigitDown - (rects[0].boundingRect().height + rects[2].boundingRect().height);
      if(DigitUp<0)
      DigitUp = 0;
      Mat ROIOfDigits = img1(Range(DigitUp,DigitDown),Range(DigitLeft,DigitRight));
      if(!DigitThread(ROIOfDigits, mst.currentDigits,s))
      {
        mst.currentDigits.clear();
        mst.currentMNIST.clear();
        continue;
      }
      string haha;
      for(int i = 0; i< 5;i++)
      {
        haha  = haha +" "+to_string(mst.currentDigits[i]);
      }
      putText(img,haha,Point(400,100),
             FONT_HERSHEY_SIMPLEX, 1 , Scalar(0,255,0),3);
      putText(img,to_string(cIdx),Point(100,100),
             FONT_HERSHEY_SIMPLEX, 1 , Scalar(0,255,0),3);
      imshow("a",img);
      waitKey(1);
      //int hitIndex = mst.whichToShootSemiAuto(myfile, s.imgCPSetting.hitNumber);
      myfile<<cIdx<<endl;
      int hitIndex = mst.whichToShootAuto(myfile);
      myfile<<"*************"<<endl;
      if (hitIndex == -1) {
        mst.Fail();
        continue;
      }
      clock_t end2 = clock();
      Rect t = rects[MR.mnistLabels[hitIndex]].boundingRect();
      AngleSolver ag;
      ag.setDistortionCoefficients(s);
      ag.setCameraMAtrix(s);
      ag.setRealWorldTargetS(s);
      vector<Point2f> input;
      input.push_back(Point2f(t.x, t.y));
      input.push_back(Point2f(t.x, t.y + t.height));
      input.push_back(Point2f(t.x + t.width, t.y));
      input.push_back(Point2f(t.x + t.width, t.y + t.height));
      //string filename = "MNISTRecord/pic" + to_string(cIdx) + ".png";
      // imwrite(filename,img);
      // MR.recordResults(cIdx);
      // myfile<<"fire : "<<to_string(cIdx)<<endl;
      // for(int i = 1;i<10;i++)
      // {
      //     myfile<<to_string(i)<<","<<to_string(MR.mnistLabels[i])<<" | ";
      // }
      // myfile<<endl;
      if (!ag.setImageTargetS(input, img)) {
        cout << "setImageTarget gg " << endl;
      } else {
        // myfile<<"cameraMatrix"<<ag.cameraMatrix<<endl;
        // myfile<<"distortionCoefficients"<<ag.distortionCoefficients<<endl;
        ag.getRotation_Translation_Matrix();
        // myfile<<"target input: ("<<ag.targetInImage[0].x<<" ,
        // "<<ag.targetInImage[0].y<<") ("
        // 	  <<ag.targetInImage[1].x<<" , "<<ag.targetInImage[1].y<<") ("
        // 	  <<ag.targetInImage[2].x<<" , "<<ag.targetInImage[2].y<<") ("
        // 	  <<ag.targetInImage[3].x<<" , "<<ag.targetInImage[3].y<<")
        // ("<<endl;

        ag.getPositionInfo(target.x, target.y, target.z);
        // myfile<<"target result: "<<to_string(target.x)<<"
        // "<<to_string(target.y)<<" "<<to_string(target.z)<<endl;
        rune_pub.publish(target);
        waitKey(200);
        ROS_INFO("x: %f y: %f z: %f", target.x, target.y, target.z);
        // waitKey(shootingDelay);
        ag.sendAns(img);
        waitKey(1);
        // string filename = "pic" + to_string(cIdx)+".png";
        // imwrite(filename,img);

        failure = false;
      }
      clock_t end3 = clock();
      myfile << to_string(cIdx) << " : end3 - end2"
             << (double)(end3 - end2) / CLOCKS_PER_SEC << endl;
    } else {
      // MR.reflect(cIdx);
      // waitKey(20);
      mst.Fail();
      continue;
    }

    // waitKey(20);
  }
  myfile.close();
}
