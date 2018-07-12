#include"DigitRecognizer.h"

/*
preprocessRGB(Mat& image,Mat& result)
thresholding by RGB value, red > 200 && red > 1.1 * blue
colse to eliminate tiny gaps and noise
*/
void DigitRecognizer::preprocessRGB(Mat image,Mat& result)
{
    vector<Mat> channels;
    split(image,channels); 
    Mat Red = channels.at(2);
    Mat Blue = channels.at(0);
    Mat Green = channels.at(1);
    Mat R_B = Red - Blue;
    Mat R_G = Red - Green;
    // imshow("R-B",R_B);
    // waitKey(1);
    // imshow("R-G",R_G);
    // waitKey(1);
    // result = R_B & R_G;
    // threshold(result,result,90,255,THRESH_BINARY);
    
    
    // imshow("R",Red);
    // waitKey(1);
    // imshow("G",Green);
    // waitKey(1);
    // imshow("B",Blue);
    // waitKey(1);
    double redMean = mean(Red)[0];
    double blueMean = mean(Blue)[0];
    double greenMean = mean(Green)[0];
    for(int i = 0 ; i < Red.rows; i++)                                        
    {
        for(int j = 0; j < Red.cols;j++)
        {
            if(Red.at<uchar>(i,j)>redMean*1.3 && Red.at<uchar>(i,j)>=Blue.at<uchar>(i,j) && Red.at<uchar>(i,j)>=Green.at<uchar>(i,j) && Blue.at<uchar>(i,j)<blueMean*1.2 && Green.at<uchar>(i,j)<greenMean*1.2 )
            {
                Red.at<uchar>(i,j) = 255;
            }
            else
            {
                Red.at<uchar>(i,j) = 0;
            }
        }
    }
    threshold(Red,Red,200,255,THRESH_BINARY);
   morphologyEx(result,result,MORPH_DILATE,getStructuringElement(MORPH_RECT,Size(3,3)));
    morphologyEx(result,result,MORPH_ERODE,getStructuringElement(MORPH_RECT,Size(3,3)));
    morphologyEx(result,result,MORPH_DILATE,getStructuringElement(MORPH_RECT,Size(3,3)));
    morphologyEx(result,result,MORPH_ERODE,getStructuringElement(MORPH_RECT,Size(3,3)));
    Red.copyTo(this->binary);

#if show1
    imshow("result",Red);
    waitKey(3);
#endif
    return ;
}

void DigitRecognizer:: preprocessHSV(Mat& image, Mat& result)
{
    Mat tempHSV;
	vector<Mat> hsvSplit;
	cvtColor(image, tempHSV, COLOR_BGR2HSV);
	split(tempHSV, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);
	merge(hsvSplit, tempHSV);
	inRange(tempHSV, Scalar(0,35, 10), Scalar(230, 255,255), result);
    morphologyEx(result,result,MORPH_CLOSE,getStructuringElement(MORPH_RECT,Size(7,7)));
    imshow("result",result);
    waitKey(3);
    return;
}

bool DigitRecognizer::findDigits()
{
    vector<vector<Point2i>> contours;
    contours.clear();
    vector<Vec4i> hierarchy;
    findContours(binary,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
    vector<Rect> boundRect(contours.size());
    vector<Rect> possibleTargetRects;
    vector<vector<Point>> contours_poly(contours.size());
    for(int i = 0 ; i < contours.size();i++) 
    {
        Mat a = Mat(contours[i]);
        boundRect[i] = boundingRect(Mat(contours[i]));
        
        if(boundRect[i].area() > 50 &&  boundRect[i].area() < 25000)
        {
           possibleTargetRects.push_back(boundRect[i]);
           //rectangle(binary,boundRect[i],Scalar(255,255,255));
        }
    }
    if(possibleTargetRects.size()<5)
    {
        cout<<"original possibleTargetRects.size()<5"<<endl;
        cout<< possibleTargetRects.size()<<endl;
        return false;
    }
    if(possibleTargetRects.size()==5)
    {   

        cout<<" exactly 5"<<endl;
        for(int i = 0; i<5;i++)
        {
            targets.push_back(possibleTargetRects[i]);
        }
        return true;
    }
    
}

int DigitRecognizer::recognize(Mat img)
{
    double ratio = (double)img.rows/(double)img.cols;
    if(ratio<1)
    {
        return -1;
    }
    else if(ratio>3 && ratio < 10)
    {
        return 1;
    }
    Mat row1 = img.rowRange(img.rows/3,img.rows/3 +1);    
    Mat row2 = img.rowRange(img.rows*2/3,img.rows*2/3 + 1);
    Mat col1 = img.colRange(img.cols/2,img.cols/2+1);
    int row1Count = 0;
    int row2Count = 0;
    int col1Count = 0;
    int count = 0;
    int row1Points[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    int row2Points[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    int col1Points[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    for(int i = 0; i<row1.cols -1; i++)
    {
        if(abs( img.at<uchar>(img.rows/3,i)-img.at<uchar>(img.rows/3,i+1) )>200)
        {
            row1Points[row1Count] = i;
            row1Count++;
            count++;
        }
    }
    for(int i = 0; i<row2.cols -1; i++)
    {
        if(abs(img.at<uchar>(img.rows*2/3,i)-img.at<uchar>(img.rows*2/3,i+1))>200)
        {
            row2Points[row2Count] = i;
            row2Count++;
            count++;
        }
    }
    for(int i = 0; i<col1.rows -1; i++)
    {
        if(abs(img.at<uchar>(i,img.cols/2)-img.at<uchar>(i+1,img.cols/2))>200)
        {
            col1Points[col1Count] = i;
            col1Count++;
            count++;
        }
    }
    //cout<< ": "<<row1Count << " " << row2Count << " "<< col1Count <<endl;
    if(count == 6)//possibly 7
    {
        if(row1Count == 2 && col1Count == 2 && row2Count == 2)
        {
            return 7;
        }
    }
    if(count == 8)// possibly 4
    {
        if(row1Count == 4 && col1Count == 2 && row2Count == 2)
        {
            return 4;
        }
    }
    if(count == 10)// possibly 2 3 5
    {
        if(row1Count == 2 && row2Count == 2 && col1Count == 6)
        {
            if(row1Points[0] > row1.cols/2 && row2Points[1] < row2.cols/2)
            {
                return 2;
            }
            if(row1Points[0] > row1.cols/2 && row2Points[0] > row2.cols/2)
            {
                return 3;
            }
            if(row1Points[1] < row1.cols/2 && row2Points[0] > row2.cols/2)
            {
                return 5;
            }
        }
    }
    if(count == 12)//possibly 6 9
    {
        if(row1Count == 2 && row2Count == 4 && col1Count == 6)
        {
            if(row1Points[1] < row1.cols/2 && row2Points[1] < row2.cols/2 && row2Points[2] > row2.cols/2 )
            {
                return 6;
            }
        }
        if(row1Count == 4 && row2Count == 2 && col1Count == 6)
        {
            if(row1Points[1] < row1.cols/2 && row1Points[2] > row2.cols/2 && row2Points[0] > row2.cols/2 )
            {
                return 9;
            }
        }

    }
    if(count == 14)// possibly 8
    {
        if(row1Count == 4 && row2Count == 4 && col1Count == 6)
        {
            return 8;
        }
    }

    return -1;
}


bool DigitRecognizer::getAns()
{
	float data[] = {1, 0.1, 0,  0, 1, 0};
	Mat affine(2, 3, CV_32FC1, data);
    //warpAffine(digitTemplateImgs.at(i), digitTemplateImgs.at(i), affine, digitTemplateImgs.at(i).size());
    
    for (int i = 0;i < 5; i++)
	{
		// TODO:
        // wrapperspective
        Mat a ;
        if(targets[i].x > binary.size().width || targets[i].y > binary.size().height)
        {
            ans[i] = -2;
            continue;
        }
        if(targets[i].x + targets[i].width > binary.size().width || targets[i].y + targets[i].height > binary.size().height)
        {
            ans[i] = -2;
            continue;
        }
        morphologyEx(binary(targets[i]),a,MORPH_ERODE,getStructuringElement(MORPH_RECT,Size(3,3)));
        ans[i] = recognize(a);
	}
    for(int i = 0; i<5;i++)
    {
        if(ans[i]==-1)
        {
            return false;
        }
    }
    return true;
}

void DigitRecognizer:: recordResults(int idx)
{
	std::ofstream myfile;
	string filename = "./DigitRecord/scores "+ to_string(idx)+".txt";
  	myfile.open (filename);
	myfile<< " -------------------"<<idx<<"------------"<<endl;
  	for (int i = 0; i< 5; i++)
	{
		myfile << ans[i] << "  ";
	}
  	myfile.close();
    string picName = "./DigitRecord/pic"+ to_string(idx) + ".png";
    imwrite(picName,binary);
	return;

}

