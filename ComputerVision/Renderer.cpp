/*
 * Renderer.cpp
 *
 *  Created on: 18.01.2013
 *      Author: ick
 */

#include "Renderer.h"
#include <stdio.h>
#include <exception>
#include <cv.h>
#include <GL/glut.h>

using namespace cv;
using namespace std;


int i = 0;		//TODO remove

int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cout << "USAGE: argv[0]=InputVideo-FILENAME,  arg[1]=ObjectPositionData-Filename"<< std::endl;
		return EXIT_SUCCESS;
	}

	string video_filename = std::string(argv[1]);
	string data_filename = std::string(argv[2]);
	vector<Mat> frames;
	VideoCapture cap(video_filename);
	for (double i = 0; i < cap.get(CV_CAP_PROP_FRAME_COUNT); i++) {

		Mat frame;
		cap >> frame;
		frames.push_back(frame.clone());
	}

	Renderer renderer(video_filename + "_render2.avi", cap, data_filename, true);
	renderer.render(frames, frames);
}

Renderer::Renderer(std::string filename2, cv::Size size2, double framerate2, std::string initFilename, bool preview2) {
	readObjectAndLightData(initFilename);
	init(filename2, size2, framerate2, preview2);
}

Renderer::Renderer(std::string filename, cv::VideoCapture inputVideo, std::string initFilename, bool preview) {
	Size s = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
	double framerate = inputVideo.get(CV_CAP_PROP_FPS);

	readObjectAndLightData(initFilename);
	init(filename, s, framerate, preview);
}

Renderer::~Renderer() {
}

void Renderer::init(std::string filename2, cv::Size size2, double framerate2, bool preview2) {
	preview = preview2;
	videoSize = size2;
	filename = filename2;
	framerate = framerate2;
	format = CV_FOURCC('D', 'I', 'V', 'X');

	if (preview) {
		int size = 0;
		glutInit(&size, NULL);
		//glutInitWindowPosition(0,0);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		glutInitWindowSize(videoSize.width, videoSize.height);
		glutCreateWindow("MatchMover");
		glClearColor(0.0, 0.0, 0.0, 1.0);
	}

	//init light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	GLfloat ambient[]={0,0,0,1};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	GLfloat diffuse[]={1,1,1,1};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	GLfloat specular[]={1,1,1,1};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	//TODO glLightModel with 0.2,0.2,0.2,1

	glEnable(GL_LIGHTING);	//Enable light
	glEnable(GL_LIGHT0);

	//init material
	GLfloat specularMaterial[]={1,1,1,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMaterial);
	GLfloat emissionMaterial[]={0,0,0,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissionMaterial);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); //let vertex color override material properties
	glEnable(GL_COLOR_MATERIAL);
}

void Renderer::readObjectAndLightData(std::string filename) {
	try {
		FileStorage fs(filename, FileStorage::READ);
		FileNode node = fs["object"];
		double object_x = (double) node["x"];
		double object_y = (double) node["y"];
		double object_z = (double) node["z"];
		scale = (double) node["scale"];

		objectPosition = Point3d(object_x, object_y, object_z);

		node = fs["light"];
		lightPosition[0] = (double) node["x"];
		lightPosition[1] = (double) node["y"];
		lightPosition[2] = (double) node["z"];
		//light = Point3d(light_x, light_y, light_z);
	} catch (Exception& e) {
		cout << "Object and light position data could not found/read" << endl;
	}
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

	//render object
	renderObject(background/*, cameraposition*/);

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
	glOrtho(0, videoSize.width, 0, videoSize.height, -500, 500); //set ortho view
	//gluPerspective(45.0,videoSize.width/videoSize.height,-100,1000.0);
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
	//glDisable(GL_TEXTURE_2D);

	//Draw background - (slow solution)
	glDrawPixels(videoSize.width, videoSize.height, GL_BGR, GL_UNSIGNED_BYTE, background.data);


	glPushMatrix();
	//TODO set new cameraposition - modelview
	glRotatef(i++, 1.0f, 1.0f, 0.2f); //TODO

	//TODO update light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);


	//glRotatef(i, 1.0f, 1.0f, 0.0f);
	//draw plane for shadow
	//glBegin(GL_QUADS);
	//glColor4f(0.75, 0.75, 0.75, 0.2);
	//glVertex3f(0.0f, 0.0f, -50.0f);
	//glVertex3f(0.0f, 0.0f, 50.0f);
	//glVertex3f(videoSize.width, 0.0f, 50.0f);
	//glVertex3f(videoSize.width, 0.0f, -50.0f);
	//glEnd();

	//draw 3D-Object
	glPushMatrix();
	glColor4f(0.75, 0.75, 0.75, 1.0);
	glTranslated(objectPosition.x, objectPosition.y, objectPosition.z); //Position in the room
	//glMultMatrixf

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
