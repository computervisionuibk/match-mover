/*
 * Renderer.cpp
 *
 *  Created on: 18.01.2013
 *      Author: ick
 */

#include "Renderer.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cv.h>
#include <GL/glut.h>

using namespace cv;
using namespace std;

//TODO remove
int i = 0;

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cout << "USAGE: " << argv[0] << "FILENAME" << std::endl;
		return EXIT_SUCCESS;
	}

	string filename = std::string(argv[1]);
	cout << filename << endl;
	vector<Mat> frames;
	VideoCapture cap(filename);
	for (double i = 0; i < cap.get(CV_CAP_PROP_FRAME_COUNT); i++) {

		Mat frame;
		cap >> frame;
		frames.push_back(frame.clone());
	}

	Renderer renderer(filename + "_render2.avi", cap, "/home/ick/test.yml", true);
	renderer.render(frames, frames);
}

Renderer::Renderer(std::string filename2, cv::Size size2, double framerate2, std::string initFilename, bool preview2) {
	init(filename2, size2, framerate2, preview2);
	readObjectAndLightData(initFilename);
}

Renderer::Renderer(std::string filename, cv::VideoCapture inputVideo, std::string initFilename, bool preview) {
	Size s = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
	double framerate = inputVideo.get(CV_CAP_PROP_FPS);

	init(filename, s, framerate, preview);
	readObjectAndLightData(initFilename);
	cout << light.x << " " << light.y << " " << light.z << endl;
}

Renderer::~Renderer() {
}

void Renderer::init(std::string filename2, cv::Size size2, double framerate2, bool preview2) {
	preview = preview2;
	videoSize = size2;
	filename = filename2;
	framerate = framerate2;
	format = CV_FOURCC('D', 'I', 'V', 'X');

	//TODO init object
	//TODO init lightposition

	if (preview) {
		int size = 0;
		glutInit(&size, NULL);
		//glutInitWindowPosition(0,0);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		glutInitWindowSize(videoSize.width, videoSize.height);
		glutCreateWindow("MatchMover");
		glClearColor(0.0, 0.0, 0.0, 1.0);
	}
}

void Renderer::readObjectAndLightData(std::string filename) {
	FileStorage fs(filename, FileStorage::READ);
	FileNode node = fs["object"];
	double object_x = (double) node["x"];
	double object_y = (double) node["y"];
	double object_z = (double) node["z"];
	scale = (double) node["scale"];

	object = Point3d(object_x, object_y, object_z);
	cout << object_x << " " << object_y << " " << object_z << endl;

	node = fs["light"];
	double light_x = (double) node["x"];
	double light_y = (double) node["y"];
	double light_z = (double) node["z"];

	light = Point3d(light_x, light_y, light_z);
	cout << light_x << " " << light_y << " " << light_z << endl;
	cout << light.x << " " << light.y << " " << light.z << endl;
}

bool Renderer::openOutputFile() {
	videoWriter.open(filename, format, framerate, videoSize, true);
	if (videoWriter.isOpened())
		return true;
	else
		return false;
}

void Renderer::render(std::vector<cv::Mat>& video, std::vector<cv::Mat> cameramovement) {
	//open VideoWriter for output-video
	openOutputFile();

	for (vector<Mat>::iterator it = video.begin(); it != video.end(); ++it) {
		renderFrame(*it);
		writeNextFrame(*it);

		char c;
		if ((c = waitKey(100)) >= 0) {
			cout << "key pressed: " << c << endl;
			if (c == 'a')
				i += 20;
			//OnKeyPress(c,0,0);
		}
	}
}

void Renderer::renderFrame(cv::Mat& background) {
	cv::Mat renderedImage(videoSize, CV_8UC3);
	cv::flip(background, background, 0); //flip image - OpenGL and OpenCV image-data's are different

	//set camera

	//render object
	renderObject(background);

	//getOutput
	glReadPixels(0, 0, videoSize.width, videoSize.height, GL_BGR, GL_UNSIGNED_BYTE, (uchar*) renderedImage.data); //grab OpenGL frame buffer and store it in OpenCV image
	cv::flip(renderedImage, renderedImage, 0); //flip image
	background = renderedImage;
}

void Renderer::renderObject(cv::Mat& background) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, videoSize.width, 0, videoSize.height, -100, 500); //set ortho view
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//Draw a textured quad - for the background
	//glColor4f(1, 1, 1, 1);

	//glEnable(GL_TEXTURE_2D);
	//glBegin(GL_QUADS);
	//	glTexCoord2f(0.0f, 0.0f);
	//	glVertex2f(0.0f, 0.0f);
	//	glTexCoord2f(1.0f, 0.0f);
	//	glVertex2f(VIEWPORT_WIDTH, 0.0f);
	//	glTexCoord2f(1.0f, 1.0f);
	//	glVertex2f(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	//	glTexCoord2f(0.0f, 1.0f);
	//	glVertex2f(0.0f, VIEWPORT_HEIGHT);
	//glEnd();

	glDrawPixels(videoSize.width, videoSize.height, GL_BGR, GL_UNSIGNED_BYTE, background.data);

	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	//glRotatef(i, 1.0f, 1.0f, 0.0f);
	//draw plane for shadow
	glBegin(GL_QUADS);
	glColor4f(0.75, 0.75, 0.75, 0.2);
	glVertex3f(0.0f, 0.0f, -50.0f);
	glVertex3f(0.0f, 0.0f, 50.0f);
	glVertex3f(videoSize.width, 0.0f, 50.0f);
	glVertex3f(videoSize.width, 0.0f, -50.0f);
	glEnd();

	//draw 3D-Object
	glPushMatrix();
	glColor4f(0.75, 0.75, 0.75, 1.0);
	glTranslated(object.x, object.y, object.z); //Position in the room
	//glMultMatrixf
	glRotatef(i++, 0.1f, 1.0f, 0.2f);			//TODO

	glScalef(scale, scale, scale); //TODO
	glutSolidTeapot(1);
	glPopMatrix();
	glPopMatrix();

	glFlush();

	glutSwapBuffers();
}

void Renderer::writeNextFrame(cv::Mat& frame) {
	if (videoWriter.isOpened()) {
		videoWriter.write(frame);
	} else {
		std::cout << "Error: Video not open" << std::endl;
	}
}
