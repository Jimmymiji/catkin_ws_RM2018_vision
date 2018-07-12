#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>

using namespace cv;
using namespace std;


int thresh = 50, N = 5;

double angle(Point pt1, Point pt2, Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


void findSquares(const Mat& image, vector<vector<Point> >& squares)
{
    squares.clear();
   
    
    Mat timg(image);
    medianBlur(image, timg, 5);
    Mat gray0(timg.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    
    for (int c = 0; c < 3; c++)
    {
        int ch[] = { c, 0 };
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        
        for (int l = 0; l < N; l++)
        {
            
            if (l == 0)
            {
            
                Canny(gray0, gray, 5, thresh, 5);
                dilate(gray, gray, Mat(), Point(-1, -1));
            }
            else
            {
                gray = gray0 >= (l + 1) * 255 / N;
            }
            imshow("canny",gray);
            // find contours and store them all as a list
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
            imshow("gay",gray);
            vector<Point> approx;

            // test each contour
            for (size_t i = 0; i < contours.size(); i++)
            {
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                if (approx.size() == 4 &&
                    fabs(contourArea(Mat(approx))) > 1000 &&
                    isContourConvex(Mat(approx)))
                {
                    double maxCosine = 0;

                    for (int j = 2; j < 5; j++)
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }
                    if (maxCosine < 0.3)
                        squares.push_back(approx);
                }
            }
        }
    }
}

void findSquaresBinary(const Mat& image,  vector<vector<Point> >& squares)
{
    
    vector<vector<Point> > contours;
    findContours(image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);
    vector<Point> approx;

            // test each contour
            for (size_t i = 0; i < contours.size(); i++)
            {
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                if ( (approx.size() == 4 || approx.size() == 3) &&
                    fabs(contourArea(Mat(approx))) > 1000 &&
                    isContourConvex(Mat(approx)))
                {
                    double maxCosine = 0;

                    for (int j = 2; j < 5; j++)
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }
                    if (maxCosine < 0.3)
                        squares.push_back(approx);
                }
            }
}

double maxRectArea = 5000;
double minRectArea = 1000;
double maxHWRatio = 2;
double minHWRatio = 0.6;
void findRects(Mat image,vector<vector<Point>>& squares)
{
    squares.clear();
    vector<vector<Point>> contours;
    findContours(image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);
    for(int i = 0;i<contours.size();i++)
    {
        Rect temp = boundingRect(contours[i]);
        double HWRatio = (double)temp.height/(double)temp.width;
        if(HWRatio>minHWRatio && HWRatio <maxHWRatio)
        {
            if(temp.area()>minRectArea && temp.area()<maxRectArea)
            {
                    double Mean = mean(image(temp))[0];
                    if(Mean > 80)
                    {
                        squares.push_back(contours[i]);
                    }
            }
        }
        // cout<< i <<": height: "<<temp.height <<" width: "<<temp.width<<" ration: "<<HWRatio<<" size: "<<temp.area()<<endl;
    }
    // cout<<"possible rects : "<<squares.size()<<endl;
}

// the function draws all the squares in the image
static void drawSquares(Mat image, const vector<vector<Point> >& squares)
{
    for (size_t i = 0; i < squares.size(); i++)
    {
        const Point* p = &squares[i][0];

        int n = (int)squares[i].size();
        //dont detect the border
        if (p->x > 3 && p->y > 3)
            polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, CV_AA);
        
        RotatedRect minRect = minAreaRect(squares[i]);
        int h = minRect.size.height;
        int w = minRect.size.width;
        string text =  to_string(w) + " , "+ to_string(h);
        putText(image,text,minRect.center, FONT_HERSHEY_SIMPLEX, 1 , Scalar(255,255,255));
        

    }
    imshow("Square Detection Demo", image);
}

bool ascendingY(const RotatedRect& a,const RotatedRect& b) {return (a.center.y<b.center.y);}
bool ascendingX(const RotatedRect& a,const RotatedRect& b) {return (a.center.x<b.center.x);}
bool descendingY(const RotatedRect& a,const RotatedRect& b) {return (a.center.y>b.center.y);}
bool descendingX(const RotatedRect& a,const RotatedRect& b) {return (a.center.x>b.center.x);}

struct RectWithDist
{
    RotatedRect r;
    float d;
};

bool checkRects(Mat& img, vector<vector<Point> >& squares,vector<RotatedRect>& rects)
{
    // too few squares
    if(squares.size()<9) 
    {
        // cout<<"squares.size()<9"<<endl;
        return false;
    }
    rects.clear();
    //cout<< "_________________________"<<endl;
    for(int i = 0; i < squares.size();i++) 
    {
        RotatedRect minRect = minAreaRect(squares[i]);
        //cout<< "size of "<< i << "  "<<minRect.size.width << " , "<<minRect.size.height<<endl;
        if(minRect.size.height<20||minRect.size.width<20)//||minRect.size.width>minRect.size.height*0.8)
        {
            continue;
        }
        else
        {
            rects.push_back(minRect);
        }
    }
    //cout<< "_________________________"<<endl;
    if(rects.size()<9)
    {
        //cout<<"------rects.size()<9"<<endl;
    }
    sort(rects.begin(),rects.end(),ascendingX);
    vector<RotatedRect>::iterator p = rects.begin();
   
    for(;p<rects.end();)
    {
        if(abs(p->center.x - (p+1)->center.x)<5 && abs(p->center.y - (p+1)->center.y)<5)
        {
            p = rects.erase(p);                                                       
        }
        else
        {
            p++;
        }
    }
    if(rects.size()<9)
    {
      //  cout<<"rects.size()<9"<<endl;
    }
	if(rects.size()>9)	
    {
        std::sort(rects.begin(),rects.end(), [](RotatedRect &r1, RotatedRect &r2) { return r1.size.area() > r2.size.area(); });
        rects.erase(rects.begin()+9,rects.end());
    }
    // possibly what we want
    if(rects.size()==9)
    {
        /*
               arrange the rects, hopefully to have:
               0 1 2
               3 4 5
               6 7 8
        */
        sort(rects.begin(),rects.end(),ascendingY);                
        sort(rects.begin(),rects.begin()+3,ascendingX);          
        sort(rects.begin()+3,rects.begin()+6,ascendingX);
        sort(rects.begin()+6,rects.begin()+9,ascendingX);
        // cout<<"1 , 2 , 3"<<endl;
        // cout<<"4 , 5 , 6"<<endl;
        // cout<<"7 , 8 , 9"<<endl;
        cout<<"-----------------"<<endl;
        return true;
    }
    return false;
    
}
