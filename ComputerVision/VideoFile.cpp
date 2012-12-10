#include "VideoFile.h"
#include <opencv2/opencv.hpp>
#include <stdlib.h>

using namespace std;
using namespace cv;

VideoFile::VideoFile(string fileName)
{
	cout<<"Creating the loader of video, with path: "<<fileName<<"."<<endl;
	_fileName = fileName;
}


vector<Mat> VideoFile::getVideo(){
	vector<Mat> frames;

	VideoCapture cap(_fileName);

	for(double i = 0; i < cap.get(CV_CAP_PROP_FRAME_COUNT); i++){
		
		Mat frame;
		cap >> frame;
		
		frames.push_back(frame.clone());
	}

	return frames;
}