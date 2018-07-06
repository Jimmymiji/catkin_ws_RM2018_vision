#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <iostream>  
#include <cmath>
class LRBlock
{
    Mat binary;
    public:
    void preprocessRGB(Mat img)
    {
        vector<Mat> channels;
        split(img,channels); 
        Mat Red = channels.at(2);
        Mat Blue = channels.at(0);
        Mat Green = channels.at(1);
        Mat B_R = Blue - Red;
        Mat B_G = Blue - Green;
        Mat result;
        // imshow("R-B",B_R);
        // waitKey(1);
        // imshow("R-G",B_R);
        // waitKey(1);
        // result = B_R & B_G;
        // imshow("result",result);
        result = B_R & B_G;
        threshold(result,result,90,255,THRESH_BINARY);
        // imshow("R",Red);
        // waitKey(1);
        // imshow("G",Green);
        // waitKey(1);
        // imshow("B",Blue);
        // waitKey(1);
        result.copyTo(binary);
        return ;
    }

    int countBlueBlock(Mat img)
    {
        preprocessRGB(img);
        vector<vector<Point>> contours;
        int blueBoxCount = 0;
        findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);
        // cout<<"contours size "<<contours.size()<<endl;
        for(int i = 0;i<contours.size();i++)
        {
            Rect temp = boundingRect(contours[i]);
            double HWRatio = (double)temp.height/(double)temp.width;
            if(HWRatio>minHWRatio && HWRatio <maxHWRatio)
            {
                if(temp.area()>minRectArea && temp.area()<maxRectArea)
                {
                   blueBoxCount++;
                }
            }
            //cout<< i <<": height: "<<temp.height <<" width: "<<temp.width<<" ration: "<<HWRatio<<" size: "<<temp.area()<<endl;
        }
        if(blueBoxCount%2 == 0)
        {
            return blueBoxCount/2;
        }
        else
        {
            return (blueBoxCount+1)/2;
        }
    }
};