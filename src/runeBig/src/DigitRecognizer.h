#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <iostream>  
#include <cmath>
#include <fstream>
#include "define.hpp"
using namespace std;
using namespace cv;

class DigitRecognizer
{
    public:
   // 5 digit number result;
    Mat binary; // binary image after preprocessing
    int left , right , low;
    vector<Rect> targets;
    int ans[5]  = {-1,-1,-1,-1,-1};
    void preprocessHSV(Mat& image, Mat& result);
    void preprocessRGB(Mat image,Mat& result);
    bool findDigits();
    int  recognize(Mat img);
    bool getAns();
    void recordResults(int idx);

};
