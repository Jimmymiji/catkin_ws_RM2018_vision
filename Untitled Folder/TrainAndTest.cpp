// TrainAndTest.cpp

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/ml/ml.hpp>
#include<string>

#include<iostream>
#include<sstream>
using namespace std;
using namespace cv;


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

int predict(cv::Ptr<cv::ml::KNearest> kNearest, Mat matTestingNumbers){
	std::vector<ContourWithData> allContoursWithData;
	std::vector<ContourWithData> validContoursWithData;

	if (matTestingNumbers.empty()) {
		std::cout << "error: image not read from file\n\n";
		return(0);
	}
	cv::Mat matInRange;
	cv::Mat matBlurred;
	cv::Mat matThresh;
	cv::Mat matThreshCopy;

	inRange(matTestingNumbers, Scalar(70, 120, 0), Scalar(255, 255, 255), matInRange);
	cout <<"inrange complete"<<endl;
	Mat structer = getStructuringElement(MORPH_ELLIPSE, Size(10, 10));
	Mat temp1;
	Mat temp2;
	erode(matInRange, temp1, structer);
	dilate(temp1, matBlurred,structer);
	imshow("After DE", matBlurred);
	cv::resize(matBlurred, temp2, cv::Size(140, 140));
	cout <<"Resize complete"<<endl;
	bitwise_not(temp2, temp2);
	cv::adaptiveThreshold(temp2, matThresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 11, 2);
	cout <<"AT complete"<<endl;
	imshow("matThresh",matThresh);
	matThreshCopy = matThresh.clone();
	std::vector<std::vector<cv::Point> > ptContours;
	std::vector<cv::Vec4i> v4iHierarchy;
	cv::findContours(matThreshCopy, ptContours,v4iHierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < ptContours.size(); i++) {
		ContourWithData contourWithData;
		contourWithData.ptContour = ptContours[i];
		contourWithData.boundingRect = cv::boundingRect(contourWithData.ptContour);
		contourWithData.fltArea = cv::contourArea(contourWithData.ptContour);
		allContoursWithData.push_back(contourWithData);
	}

	for (int i = 0; i < allContoursWithData.size(); i++) {
		if (allContoursWithData[i].checkIfContourIsValid()) {
			validContoursWithData.push_back(allContoursWithData[i]);
		}
	}
	std::sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByArea);
	ContourWithData Biggest = validContoursWithData[0];
	std::string strFinalString;
	cv::rectangle(matTestingNumbers,
		Biggest.boundingRect,
		cv::Scalar(0, 255, 0),
		2);
	cv::Mat matROI = matThresh(Biggest.boundingRect);

	cv::Mat matROIResized;
	cv::resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));
	imshow("matROIResized", matROIResized);
	cv::Mat matROIFloat;
	matROIResized.convertTo(matROIFloat, CV_32FC1);
	cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);
	cv::Mat matCurrentChar(0, 0, CV_32F);
	kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);
	float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);
	strFinalString = strFinalString + char(int(fltCurrentChar));

	

	std::cout << "\n\n" << "numbers read = " << strFinalString << "\n\n";

	cv::imshow("matTestingNumbers", matTestingNumbers);

	cv::waitKey(0);

}



int main(int argc, char* argv[]) {
	
	// cv::Mat matClassificationInts;

	// cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);

	// if (fsClassifications.isOpened() == false) {
	// 	std::cout << "error, unable to open training classifications file, exiting program\n\n";
	// 	return(0);
	// }

	// fsClassifications["classifications"] >> matClassificationInts;
	// fsClassifications.release();

	// cv::Mat matTrainingImagesAsFlattenedFloats;

	// cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);

	// if (fsTrainingImages.isOpened() == false) {
	// 	std::cout << "error, unable to open training images file, exiting program\n\n";
	// 	return(0);
	// }

	// fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;
	// fsTrainingImages.release();

	// cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());
	// kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);
	
	//Initialize knn
	cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());
	if(!loadData(kNearest))
	{
		return 1;
	}
	cout << "loadData done" << endl;


	//predict all pictures
	for (int i = 0; i < 10; i++)
	{
		string target = to_string(i);
		target.append(".jpg");
		Mat frame = imread(target);
		predict(kNearest, frame);
	}

	return 0;
}
