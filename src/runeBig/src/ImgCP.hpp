#pragma once
#include "opencv2/opencv.hpp"


class ImgCP {
	public:
		ImgCP(char mode, char* path)
		{
			videoPath = path;
			isVideoMode = mode == 'v';
            this->cameraNumber = path;
		}
		void ImageProducer();
		void ImageConsumer(int argc, char** argv);
	private:
		const char* videoPath;
		char* cameraNumber;
		bool isVideoMode;
};