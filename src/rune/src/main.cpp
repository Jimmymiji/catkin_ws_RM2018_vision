#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <cmath>
#include "Settings.h"
#include <thread>

//#if PT
//#include "PtImgCP.hpp"
//#else
#include "ImgCP.hpp"
//#endif

using namespace std;
using namespace cv;
int RiLowH = 0;
int RiHighH = 35;
int RiLowS =10;
int RiHighS = 230;
int RiLowV = 250;
int RiHighV = 255;
int main(int argc, char** argv)
{
    Settings s("setting.xml","4.yml");
    if(!s.load())
    {
	cout<<"where is my setting file?"<<endl;
	return -1;
    }

    ImgCP imgCP(argv[1][0], argv[2]);
    cout << "imgCP initialized" << endl;
	thread t1(&ImgCP::ImageProducer, imgCP); // pass by reference
	thread t2(&ImgCP::ImageConsumer, imgCP, argc, argv);
    t1.join();
	t2.join();
    // #endif

}
