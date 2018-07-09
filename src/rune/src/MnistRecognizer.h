#ifndef MN
#define MN
#include "tiny_dnn/tiny_dnn.h"
#include <opencv2/opencv.hpp>
#include "define.hpp"

using namespace tiny_dnn;
using namespace tiny_dnn::activation;
using namespace std;
using namespace cv;

class MnistRecognizer
{
public:
	MnistRecognizer(const string& dictionary = "LeNet-model");
	Point getCenter(int label);
	int getLabel(int index);
	~MnistRecognizer();	
	vector<Mat> mnistImgs;
	int recognize( Mat& img);
	vector<pair<double, int> > recognize_primary( Mat& img);
	Mat kmeanPreprocess( Mat& inputImg);
	bool fitMnist( Mat& inputImg, Mat& resImg);
	bool classify();
	bool classify2();
	void clear();
	void recordResults(int idx);
	void M2m();
	network<sequential> nn;
 	vector<vector<pair<double,int>>> scores;// 9（class） x 9(scores of one img for each class ) x <score,label>
    map<int, int> mnistLabels; // <label,index> 
    map<int,pair<double,int>> MNISTLabels; // <label,<score,index>>
	RNG rng; // random number generator
	string Model_Path ;

};
#endif