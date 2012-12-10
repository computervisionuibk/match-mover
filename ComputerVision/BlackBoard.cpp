#include "BlackBoard.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "VideoFile.h"

using namespace std;

BlackBoard::BlackBoard(string moviePath):_video(moviePath)
{
}

void BlackBoard::load()
{
	
}

void BlackBoard::start()
{
	cout<<"Getting the frames from video."<<endl;
	vector<Mat> frames = _video.getVideo();
	
	Mat first, second;

	if(frames.size() >3){
		first = frames[0];
		second = frames[1];
	}else if (frames.size() > 2){
		first = frames[0];
		second = frames[1];
	}else{
		throw "Getting the frames was imposible.";
	}
	
	/*vector<Mat>::const_iterator iterator = frames.begin();
	first = (*iterator);
	iterator ++;
	second = (*iterator);*/
	

	Mat homography = _homography.calculateHomography(first, second);

	_homography.showMathces(0.5);
}

BlackBoard::~BlackBoard()
{

}
