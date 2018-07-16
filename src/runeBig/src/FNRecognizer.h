#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/ml/ml.hpp>
#include<string>
#include<iostream>
#include<sstream>

const int MIN_CONTOUR_AREA = 100;
const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;
class ContourWithData {
public:
	std::vector<cv::Point> ptContour;
	cv::Rect boundingRect;
	float fltArea;

	bool checkIfContourIsValid() {
		if (fltArea < MIN_CONTOUR_AREA) return false;
		return true;
	}

	static bool sortByArea(const ContourWithData& cwdBig, const ContourWithData& cwdSmall) {
		return(cwdBig.fltArea > cwdSmall.fltArea);
	}

};

bool loadData(cv::Ptr<cv::ml::KNearest> & kNearest)
{
	cv::Mat matClassificationInts;

	cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);

	if (fsClassifications.isOpened() == false) {
		std::cout << "error, unable to open training classifications file, exiting program\n\n";
		return false;
	}

	fsClassifications["classifications"] >> matClassificationInts;
	fsClassifications.release();

	cv::Mat matTrainingImagesAsFlattenedFloats;

	cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);

	if (fsTrainingImages.isOpened() == false) {
		std::cout << "error, unable to open training images file, exiting program\n\n";
		return false;
	}

	fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;
	fsTrainingImages.release();

	kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);
	return true;
}

class FNRecognizer
{
    public:
    vector<Mat> FNImages;
    vector<int> lables;
    map<int,int> relations;  // label , index

    int predict(Ptr<ml::KNearest> kNearest, Mat matTestingNumbers){
	vector<ContourWithData> allContoursWithData;
	vector<ContourWithData> validContoursWithData;
	if (matTestingNumbers.empty()) {
		cout << "error: image not read from file\n\n";
		return -1;
	}
	Mat matInRange;
	Mat matInRangeResized;
	Mat matBlurred;
	Mat matThresh;
	Mat matThreshCopy;

	inRange(matTestingNumbers, Scalar(120, 120, 0), Scalar(255, 255, 255), matInRange);
	//resize first
	resize(matInRange, matInRangeResized, Size(RESIZED_IMAGE_WIDTH *2, RESIZED_IMAGE_HEIGHT *2), 0, 0, INTER_NEAREST);
	Mat structer = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	Mat temp1;
	erode(matInRangeResized, temp1, structer);
	dilate(temp1, matBlurred,structer);
	resize(matBlurred, matThresh, Size(140, 140));
	matThreshCopy = matThresh.clone();
	vector<vector<Point> > ptContours;
	vector<Vec4i> v4iHierarchy;
	findContours(matThreshCopy, ptContours,	v4iHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < ptContours.size(); i++) {
		ContourWithData contourWithData;
		contourWithData.ptContour = ptContours[i];
		contourWithData.boundingRect = boundingRect(contourWithData.ptContour);
		contourWithData.fltArea = contourArea(contourWithData.ptContour);
		allContoursWithData.push_back(contourWithData);
	}

	for (int i = 0; i < allContoursWithData.size(); i++) {
		if (allContoursWithData[i].checkIfContourIsValid()) {
			validContoursWithData.push_back(allContoursWithData[i]);
		}
	}
	

	if(validContoursWithData.size() > 0)
	{
		sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByArea);

		ContourWithData Biggest = validContoursWithData[0];

		string strFinalString;

		//rectarectangle(matTestingNumbers, Biggest.boundingRect, Scalar(0, 255, 0), 2);

		Mat matROI = matThresh(Biggest.boundingRect);

		Mat matROIResized;
		resize(matROI, matROIResized, Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));

		Mat matROIFloat;
		matROIResized.convertTo(matROIFloat, CV_32FC1);
		Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);
		Mat matCurrentChar(0, 0, CV_32F);
		kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);
		float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);
		
		int result = (int)fltCurrentChar - 48;

		#ifdef DEBUG_RUNE
		imshow("After InrangeResize", matInRangeResized);
		imshow("After DE", matBlurred);
		imshow("matROIResized", matROIResized);
		cout << "\n\n" << "numbers read = " << result << "\n\n";
		imshow("matTestingNumbers", matTestingNumbers);
		waitKey(0);
		#endif


		return result;
	}
	else{
		return 0;
	}

}

};