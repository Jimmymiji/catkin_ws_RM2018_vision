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
            //imshow("canny",gray);
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
            imshow("gay",gray);
            vector<Point> approx;
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
                        double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }
                    if (maxCosine < 0.3)
                        squares.push_back(approx);
                }
            }
}

double maxRectArea = 25000;
double minRectArea = 2000;
double maxHWRatio = 0.9;
double minHWRatio = 0.45;
void findRects(Mat image,vector<vector<Point>>& squares)
{
    squares.clear();
    vector<vector<Point>> contours;
    findContours(image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1);
   // cout<<"contours size "<<contours.size()<<endl;
    for(int i = 0;i<contours.size();i++)
    {
        Rect temp = boundingRect(contours[i]);
        double HWRatio = (double)temp.height/(double)temp.width;
        if(HWRatio>minHWRatio && HWRatio <maxHWRatio)
        {
            if(temp.area()>minRectArea && temp.area()<maxRectArea)
            {
                    squares.push_back(contours[i]);
            }
        }
        //cout<< i <<": height: "<<temp.height <<" width: "<<temp.width<<" ration: "<<HWRatio<<" size: "<<temp.area()<<endl;
    }
    //cout<<"possible rects : "<<squares.size()<<endl;
}

static void drawSquares(Mat image, const vector<vector<Point> >& squares)
{
    for (size_t i = 0; i < squares.size(); i++)
    {
        const Point* p = &squares[i][0];

        int n = (int)squares[i].size();
        //dont detect the border
        if (p->x > 3 && p->y > 3)
            polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, CV_AA);

        //RotatedRect minRect = minAreaRect(squares[i]);
        //int h = minRect.size.height;
        //int w = minRect.size.width;
        //string text = "size : " + to_string(w) + " , "+ to_string(h);
       // putText(image,text,minRect.center, FONT_HERSHEY_SIMPLEX, 1 , Scalar(255,255,255));


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
#if debug
        //cout<<"squares.size()<9"<<endl;
#endif
        return false;
    }
    rects.clear();
#if debug
    //cout<< "_________________________"<<endl;
#endif
    for(int i = 0; i < squares.size();i++)
    {
        RotatedRect minRect = minAreaRect(squares[i]);
#if debug
        //cout<< "size of "<< i << "  "<<minRect.size.width << " , "<<minRect.size.height<<endl;
#endif
        if(minRect.size.height<30||minRect.size.width<30)//||minRect.size.width>minRect.size.height*0.8)
        {
            continue;
        }
        else
        {
            rects.push_back(minRect);
        }
    }
#if debug
    //cout<< "_________________________"<<endl;

    // if(rects.size()<9)
    // {
    //     cout<<"------rects.size()<9"<<endl;
    // }
#endif
    sort(rects.begin(),rects.end(),ascendingX);
    vector<RotatedRect>::iterator p = rects.begin();

    // eliminate the duplicated rects recognized
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

    // too few rects
//#if debug
    // if(rects.size()<9)
    // {
    //     cout<<"rects.size()<9"<<endl;
    // }
//#endif
    if(rects.size()>0)
    {
        sort(rects.begin(),rects.end(),[](RotatedRect& a, RotatedRect& b){return a.size.area() > b.size.area();});
        double Area= rects[3].size.area()*0.8;
        for(vector<RotatedRect>::iterator p = rects.begin(); p < rects.end();)
        {
            if(p->size.area() < Area)
            {
                p = rects.erase(p);
            }
            else
            {
                p++;
            }
        }
    }
    if (rects.size() > 9)
	{
        float **dist_map = new float *[rects.size()];
        int rsize = rects.size();
		for (int i = 0; i < rsize; i++)
		{
			dist_map[i] = new float[rsize];
			for (int j = 0; j < rsize; j++)
			{
				dist_map[i][j] = 0;
			}
		}
		for (int i = 0; i < rsize; ++i)
		{
			for (int j = i + 1; j < rsize; ++j)
			{
				int dx = rects[i].center.x - rects[j].center.x;
                int dy = rects[i].center.y - rects[j].center.y;
                double d = sqrt(dx*dx + dy*dy);
				dist_map[i][j] = d;
				dist_map[j][i] = d;
			}
		}
		int center_idx = 0;
		float min_dist = 100000000;
		for (int i = 0; i < rsize; ++i)
		{
			float cur_d = 0;
			for (int j = 0; j < rsize; ++j)
			{
				cur_d += dist_map[i][j];
			}
			if (cur_d < min_dist)
			{
				min_dist = cur_d;
				center_idx = i;
			}
		}
        for (int i = 0; i < rsize; i++)
		{
			delete[] dist_map[i];
		}
		delete[] dist_map;
        rectangle(img,rects[center_idx].boundingRect(),Scalar(0,255,255),5);
#if show
        imshow("center",img);
        waitKey(1);
#endif
        vector<RectWithDist> RWD;
        for(int i=0; i<rsize; i++)
        {
            RectWithDist temp;
            int dx = rects[i].center.x - rects[center_idx].center.x;
            int dy = rects[i].center.y - rects[center_idx].center.y;
            temp.d = sqrt( dx*dx + dy*dy);
            temp.r = rects[i];
            RWD.push_back(temp);
        }
		std::sort(RWD.begin(),RWD.end(), [](RectWithDist &rwd1, RectWithDist &rwd2) { return rwd1.d < rwd2.d; });
        rects.clear();
        if(RWD.size()<9)
        {
#if debug
            //cout<<"RWD.size()<9"<<endl;
#endif
            return false;
        }
        int count = 0;
        int i = 0;
        while(count < 9 && i <RWD.size())
        {
            if(RWD[i].r.size.area() < RWD[0].r.size.area()*0.8)
            {
                i++;
                continue;
            }
            rects.push_back(RWD[i].r);
            i++;
            count++;
        }

	}
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
        return true;
    }
    return false;

}
