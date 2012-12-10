#pragma once
#include "VideoFile.h";
#include "Homography.h";

class BlackBoard
{
public:
	BlackBoard(string moviePath);
	
	void load();
	void start();
	virtual ~BlackBoard();

private:

	VideoFile  _video;
	Homography _homography;
};