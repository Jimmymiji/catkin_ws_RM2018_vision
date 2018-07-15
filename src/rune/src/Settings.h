#ifndef SET
#define SET
#include "opencv2/core/core.hpp"
#include <string>
using namespace std;
using namespace cv;

struct CameraSetting
{
	CameraSetting(const string& fn) :filename1(fn) {}
	string device = "/dev/video0";
	int exposureTime = 30;
	int width = 1280,
		height = 720,
		fps = 30;
	cv::Mat cameraMatrix = (Mat1d(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
	cv::Mat distortionMatrix = (Mat1d(1, 4) << 0, 0, 0, 0);
	string filename1;
	void read(const FileNode &node);
};

struct SmallRuneSetting
{
		int width;
		int height;
		void read(const FileNode &node);
};

struct ShooterSetting
{
		double speed;
		double Xoffset;
		double Yoffset;
		double Zoffset;
		void read(const FileNode &node);
};

struct DigitRecognizerSetting
{
	double RedMean;
	int RedThreshold;
	int maxBoundingArea;
	int minBoundingArea;
	int dyPenalty;
	int dxPenalty;
	int HighLowPenalty;
	int erodeSize;
	double one;
	double maxHWRatio;
	double minHWRatio;
	void read(const FileNode& node);
};

struct FindRectSetting
{
	double maxRectArea;
	double minRectArea;
	double maxHWRatio;
	double minHWRatio;
	int checkRectHeight;
	int checkRectWidth;
	int yOffset;
	double areaRatio;
	void read(const FileNode& node);
};

struct ImgCPSetting
{
	double mean;
	int erodeSize1;
	int erodeSize2;
	int hitNumber;
	void read(const FileNode& node);
};

class Settings
{
	public:
	Settings(const string &filename1, const string& filename2) :filename1(filename1), filename2(filename2), cameraSetting(filename2) {}
	~Settings() {}
	bool load();
	bool fileExist(const std::string &filename);	
	CameraSetting cameraSetting;
	SmallRuneSetting smallRuneSetting;
	ShooterSetting shooterSetting;
	DigitRecognizerSetting digitRecognizerSetting;
	FindRectSetting findRectSetting;
	ImgCPSetting imgCPSetting;
	std::string filename1, filename2;     // filename1 is the file for setting, filename2 is the file for seeting cameraMatrix & distortionMatrix
	
};

#endif