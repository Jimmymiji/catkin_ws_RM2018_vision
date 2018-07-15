#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include "Settings.h"

using namespace std;
using namespace cv;

void CameraSetting::read(const FileNode &node) //Read serialization for this class
{
	node["exposureTime"] >> exposureTime;
	node["width"] >> width;
	node["height"] >> height;
	node["fps"] >> fps;

	FileStorage fs1;
	fs1.open(filename1, FileStorage::READ);

	fs1["camera_matrix"] >> cameraMatrix;
	fs1["distortion_coefficients"]>> distortionMatrix;
	cout<<"camera matrix: "<<endl<<cameraMatrix;
	cout<<"distortionMatrix: "<<endl<<distortionMatrix;
	fs1.release();
}
void SmallRuneSetting::read(const FileNode &node) //Read serialization for this class
{
	node["width"] >> width;
	node["height"] >> height;
}
void ShooterSetting::read(const FileNode &node)
{
	node["speed"]>>speed;
	node["Xoffset"]>>Xoffset;
	node["Yoffset"]>>Yoffset;
	node["Zoffset"]>>Zoffset;

}

void DigitRecognizerSetting::read(const FileNode &node)
{
	node["RedMean"]>>RedMean;
	node["RedThreshold"]>>RedThreshold;
	node["maxBoundingArea"]>>maxBoundingArea;
	node["minBoundingArea"]>>minBoundingArea;
	node["dyPenalty"]>>dyPenalty;
	node["dxPenalty"]>>dxPenalty;
	node["HighLowPenalty"]>>HighLowPenalty;
	node["erodeSize"]>>erodeSize;
	node["one"]>>one;
	node["maxHWRatio"]>>maxHWRatio;
	node["minHWRatio"]>>minHWRatio;

}

void FindRectSetting::read(const FileNode &node)
{
	node["maxRectArea"]>>maxRectArea;
	node["minRectArea"]>>minRectArea;
	node["maxHWRatio"]>>maxHWRatio;
	node["minHWRatio"]>>minHWRatio;
	node["checkRectHeight"]>>checkRectHeight;
	node["checkRectWidth"]>>checkRectWidth;
	node["yOffset"]>>yOffset;
	node["areaRatio"]>>areaRatio;
}

void ImgCPSetting::read(const FileNode &node)
{
	node["mean"]>>mean;
	node["erodeSize1"]>>erodeSize1;
	node["erodeSize2"]>>erodeSize2;
	node["hitNumber"]>>hitNumber;
}
bool Settings::load()
{
	if (fileExist(filename1)&&fileExist(filename2))
	{
		cv::FileStorage fin(Settings::filename1, cv::FileStorage::READ);
		cameraSetting.read(fin["cameraSetting"]);
		smallRuneSetting.read(fin["smallRuneSetting"]);
		shooterSetting.read(fin["shooterSetting"]);
		digitRecognizerSetting.read(fin["DigitRecognizerSetting"]);
		findRectSetting.read(fin["FindRectSetting"]);
		imgCPSetting.read(fin["ImgCPSetting"]);
		cout<<"successfully load"<<endl;
		fin.release();
		return true;
	}
	else
	{
		return false;
	}
}

bool Settings::fileExist(const string &filename)
{
	struct stat buffer;
	return (stat(filename.c_str(), &buffer) == 0);
}
