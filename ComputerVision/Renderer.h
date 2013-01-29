/*
 * Renderer.h
 *
 *  Created on: 18.01.2013
 *      Author: ick
 */
#pragma once
#include <string.h>
#include <cv.h>
#include <GL/glut.h>
#include <highgui.h>


class Renderer {
public:
	Renderer(std::string filename, cv::Size size, double framerate, std::string initFilename, bool preview );
	Renderer(std::string filename, cv::VideoCapture video, std::string initFilename, bool preview);
	virtual ~Renderer();
	void render(std::vector<cv::Mat>& video, std::vector<cv::Mat> cameramovement);
private:
	cv::VideoWriter videoWriter;
	cv::Size videoSize;
	std::string filename;		//TODO remove
	double framerate;
	int format;
	bool preview;

	cv::Point3d objectPosition;
	double scale;
	float lightPosition[3];

	void init(std::string filename, cv::Size size, double framerate, bool preview);
	void readObjectAndLightData(std::string filename);
	bool openOutputFile();
	void writeNextFrame(cv::Mat& frame);
	void renderFrame(cv::Mat& background);
	void renderObject(cv::Mat& background);
};

//class Point3dWrapper{
//	Point3dWrapper(double _x,double _y,double _z, double _height, double _width):height(_height),width(_width){
//		point=cv::Point3d(_x,_y,_z,);
//	}
//	cv::Point3d point;
//	double height;
//	double width;
//};
