#include "Homography.h"
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
using namespace cv;

Homography::Homography(void):_strategy(sSURF)
{
}

void Homography::setStrategy(strategyIterestPoints newStrategy)
{
	_strategy = newStrategy;
}

Homography::strategyIterestPoints Homography::getStrategy(){
	return _strategy;
}

void Homography::showMathces(double minDist)
{
	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	vector< DMatch > good_matches;
	
	for( int i = 0; i < _descriptorsFirstScene.rows; i++ )
	{ 
		if( _matches[i].distance < minDist )
		{
			good_matches.push_back( _matches[i]); 
		}
	}
	//dibujamos para probar.
	Mat matches;
	drawMatches( _firstScene, _keyPointsFirstScene, _secondScene, _keyPointsSecondScene,
               good_matches, matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

	imshow("matches", matches);
	waitKey();
}


Mat Homography::calculateHomography(Mat firstScene, Mat secondScene)
{
	if(firstScene.empty() || secondScene.empty())
	{
		throw "Empty scenes!";
	}
	
	//Let's clear the attributes
	clearAttributes();

	// Set the images
	_firstScene = firstScene;
	_secondScene = secondScene;

	//Let's look for the interest points
	getInterestPoints();

	//Let's do the maching between the interest points
	computeMatching();

	//Finally, we are going to compute the homography
	return findHomography(_vFirstScene, _vSecondScene, CV_RANSAC, 2);
}

void Homography::clearAttributes()
{
	_keyPointsFirstScene.clear();
	_keyPointsSecondScene.clear();
	_matches.clear();
	_vFirstScene.clear();
	_vSecondScene.clear();
}

void Homography::getInterestPoints()
{
	switch (_strategy)
	{
		case sSURF:
			usingSurf();
			break;
		case sSIFT:
			usingSift();
			break;
		default:
			throw "Strategy does not defined.";
			break;
	}
}

void Homography::usingSift()
{	
	//-- Step 1: Detect the keypoints using SIFT Detector
	SIFT detector = SIFT(4, 3, 0.06, 10, 1.6);

	detector.detect(_firstScene, _keyPointsFirstScene);
	detector.detect(_secondScene, _keyPointsSecondScene);
		
	//-- Step 2: Calculate descriptors
	SiftDescriptorExtractor extractor; 
	 
	extractor.compute(_firstScene, _keyPointsFirstScene, _descriptorsFirstScene); 
	extractor.compute(_secondScene, _keyPointsSecondScene, _descriptorsSecondScene); 
}

void Homography::usingSurf()
{
	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;
	SurfFeatureDetector detector( minHessian );

	detector.detect(_firstScene, _keyPointsFirstScene);
	detector.detect(_secondScene, _keyPointsSecondScene);

	
	//-- Step 2: Calculate descriptors (feature vectors)
	SurfDescriptorExtractor extractor;
	
	extractor.compute(_firstScene, _keyPointsFirstScene, _descriptorsFirstScene); 
	extractor.compute(_secondScene, _keyPointsSecondScene, _descriptorsSecondScene); 
}

void Homography::computeMatching()
{

	// matching descriptors
	BFMatcher matcher = BFMatcher(NORM_L2, false); 
	matcher.match(_descriptorsFirstScene, _descriptorsSecondScene, _matches); 

	vector<DMatch>::iterator ite = _matches.begin();

	while(ite != _matches.end())
	{
		_vFirstScene.push_back(_keyPointsFirstScene[(*ite).queryIdx].pt);
		_vSecondScene.push_back(_keyPointsSecondScene[(*ite).trainIdx].pt);
		ite++;
	}
}

Homography::~Homography(void)
{
}
