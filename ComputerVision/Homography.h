#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;

class Homography
{
public:
	//Enum to choose the strategy for getting the interest points
	enum strategyIterestPoints{ sSURF, sSIFT };

	Homography(void);
	
	Mat calculateHomography(Mat firstScene, Mat secondScene) throw();
	
	void setStrategy(strategyIterestPoints newStrategy);
	strategyIterestPoints getStrategy();
	void showMathces(double minDist);

	~Homography(void);

private:
	//Attributes
	Mat _firstScene,		_secondScene, _descriptorsFirstScene, _descriptorsSecondScene;
	vector<KeyPoint>		_keyPointsFirstScene, _keyPointsSecondScene;
	vector<DMatch>			_matches;
	vector<Point2f>			_vFirstScene, _vSecondScene;
	strategyIterestPoints	_strategy;

	//Private Methods
	void clearAttributes();
	void getInterestPoints();
	void usingSift();
	void usingSurf();
	void computeMatching();

};