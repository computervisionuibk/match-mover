#pragma once
#include <string.h>
#include <cv.h>
#include <GL/glut.h>
#include <highgui.h>
#include "RenderObject.h"


class Renderer {
public:
	Renderer(std::string filename, cv::Size size, double framerate, std::string initFilename, bool preview );
	Renderer(std::string filename, cv::VideoCapture video, std::string initFilename, bool preview);
	virtual ~Renderer();
	void render(std::vector<cv::Mat>& video, std::vector<cv::Mat> rotation, std::vector<cv::Mat> translation);
	/*static*/ cv::Point3d selectObjectPosition(cv::Mat frame1, cv::Mat frame2, cv::Mat frame3);
private:
	cv::VideoWriter videoWriter;
	cv::Size videoSize;
	std::string filename;		//TODO remove
	double framerate;
	int format;
	bool preview;

	//RenderObject object;
	cv::Point3d objectPosition;
	double scale;
	float lightPosition[3];

	void init(std::string filename, std::string initFilename, cv::Size size, double framerate, bool preview);
	void readObjectAndLightData(std::string filename);
	bool openOutputFile();
	void writeNextFrame(cv::Mat& frame);
	void renderAndWriteFrame(cv::Mat& background, double cameraposition[16]);
	void renderScene(cv::Mat& background, double cameraposition[16]);
	double* setCameraPose(double modelViewMatrixRecoveredCamera[16], cv::Mat& rotation, cv::Mat& translation);
	//void handleInputEvents(unsigned char key, int x, int y);
};
