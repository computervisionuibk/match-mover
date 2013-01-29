#pragma once
#include <opencv2/core/core.hpp>

using namespace cv;
class VideoFile
{
public:
	//Constructor
	VideoFile(string fileName);

	//
	vector<Mat> getVideo();

private:
	string _fileName;
};

