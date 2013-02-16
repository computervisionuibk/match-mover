#pragma once
#include <string.h>
#include <cv.h>
#include <GL/glut.h>
#include <highgui.h>
#include "RenderObject.h"


class Renderer {
public:
	Renderer(std::string filename, cv::Size size, double framerate, std::string initFilename);
	Renderer(std::string filename, cv::VideoCapture video, std::string initFilename);
	virtual ~Renderer();
	void render(std::vector<cv::Mat>& video, std::vector<cv::Mat> rotation, std::vector<cv::Mat> translation);
	void setupObjectPostion(std::vector<cv::Mat>& previewVideo);
private:
	cv::VideoWriter videoWriter;
	cv::Size videoSize;
	std::string filename;		//TODO remove
	double framerate;
	int format;
	bool isPerspectiveSet;

	std::string* objectPositionFilename;

	//RenderObject object;
	cv::Point3d objectPosition;
	double scale;
	float lightPosition[3];

	void init(std::string filename, std::string initFilename, cv::Size size, double framerate);
	void readObjectAndLightData(std::string filename);
	bool openOutputFile();
	void writeNextFrame(cv::Mat& frame);
	void setupPerspective();
	void renderAndWriteFrame(cv::Mat& background, double cameraposition[16]);
	void renderScene(cv::Mat& background, double cameraposition[16]);
	double* setCameraPose(double modelViewMatrixRecoveredCamera[16], cv::Mat& rotation, cv::Mat& translation);
	void handleInputEvents();
	void showKeyInfo();
};
