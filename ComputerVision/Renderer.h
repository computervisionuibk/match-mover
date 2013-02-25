#pragma once
#include <string.h>
#include "opencv/cv.h"
#include <GL/glut.h>
#include "opencv/highgui.h"


class Renderer {
public:
	Renderer(std::string _outputVideoFilename, cv::Size videoResolution, double framerate, std::string* initObjectFilename, cv::Mat K/*kalibration*/);
	Renderer(std::string _outputVideoFilename, cv::VideoCapture video, std::string* initObjectFilename, cv::Mat K/*kalibration*/);
	virtual ~Renderer();
	void render(std::vector<cv::Mat>& inputVideo, std::vector<cv::Matx34d> cvCameraPositions);
	void setupObjectPostion(std::vector<cv::Mat>& inputVideo, std::vector<cv::Matx34d> cvCameraPositions);
	void showKeyInfo();
private:
	cv::VideoWriter videoWriter;
	std::string outputVideoFilename;
	cv::Size videoSize;
	double framerate;
	int format;
	cv::Mat K;

	bool isPerspectiveSet;

	//RenderObject object;
	std::string* objectPositionFilename;
	cv::Point3d objectPosition;
	float objectRotation[3];
	double scale;
	float lightPosition[3];

	void init(std::string filename, std::string* initFilename, cv::Size size, double framerate, cv::Mat K);
	void readObjectAndLightData();
	bool openOutputFile();
	void writeNextFrame(cv::Mat& frame);
	void setupPerspective();
	void renderAndWriteFrame(cv::Mat& background, double cameraposition[16]);
	void renderScene(cv::Mat& background, double cameraposition[16]);
	double* toGLMatrix(double modelViewMatrixRecoveredCamera[16], cv::Matx34d cvCameraPosition);
	void handleInputEvents(int framecount);
};
