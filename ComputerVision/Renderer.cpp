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
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

using namespace cv;
using namespace std;

int i = 0; //TODO remove
int frame = 0; //TODO remove
int speed = 100;
std::vector<cv::Mat> previewVideo;
void handleInputEvents(unsigned char key, int x, int y);
void keyPressed (unsigned char key, int x, int y);
//void onDisplay();

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cout << "USAGE: argv[0]=InputVideo-FILENAME [, arg[1]=ObjectPositionData-Filename]" << std::endl;
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

	//TODO select object-position

	Renderer renderer(video_filename + "_render2.avi", cap, data_filename);

	renderer.setupObjectPostion(frames);

	renderer.render(frames, frames, frames);
}

Renderer::Renderer(std::string filename2, cv::Size size2, double framerate2, std::string initFilename) {
	init(filename2, initFilename, size2, framerate2);
}

Renderer::Renderer(std::string filename, cv::VideoCapture inputVideo, std::string initFilename) {
	Size s = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
	double framerate = inputVideo.get(CV_CAP_PROP_FPS);

	init(filename, initFilename, s, framerate);
}

Renderer::~Renderer() {
}

void Renderer::init(std::string filename2, string initFilename, cv::Size size2, double framerate2) {
	readObjectAndLightData(initFilename);
	videoSize = size2;
	filename = filename2;
	framerate = framerate2;
	format = CV_FOURCC('D', 'I', 'V', 'X');
	//object = QuadObject(scale);
	isPerspectiveSet=false;


	//init SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		return;
	}
	/*const SDL_VideoInfo* videoInfo =*/ SDL_GetVideoInfo();
	if (SDL_SetVideoMode(videoSize.width, videoSize.height, 32, SDL_OPENGL | SDL_RESIZABLE) == NULL) { //create window
		cerr << "Error creating SDL window!" << endl;
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_WM_SetCaption("MatchMover", NULL);
	//Antialiasing (SDL multisampling)
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); //16x

	//int size = 0;
	//glutInit(&size, NULL);
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	//glutInitWindowSize(videoSize.width, videoSize.height);
	//glutCreateWindow("MatchMover");

	glClearColor(0.0, 0.0, 0.0, 1.0);

	//setup light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	GLfloat ambient[] = { 0, 0, 0, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	GLfloat diffuse[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	GLfloat specular[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	//TODO glLightModel with 0.2,0.2,0.2,1
	GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING); //Enable light
	glEnable(GL_LIGHT0);

	//init material
	GLfloat specularMaterial[] = { 1, 1, 1, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMaterial);
	GLfloat emissionMaterial[] = { 0, 0, 0, 1 };
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
		//if (objectPosition==NULL)
		//	Point3d(0, 0, 0);

		//lightPosition[0] = 0.0;
		//lightPosition[1] = 0.0;
		//lightPosition[2] = 0.0;
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

void Renderer::showKeyInfo(){
	cout<< "Keys:" << endl;
	cout<< " - Video-Speed -" << endl;
	cout<< "Increase: +" << endl;
	cout<< "Decrease: -" << endl;
	cout<< endl;
	cout<< " - Object -" << endl;
	cout<< "Increase - x-position: d" << endl;
	cout<< "Decrease - x-position: a" << endl;
	cout<< "Increase - y-position: w" << endl;
	cout<< "Decrease - y-position: s" << endl;
	cout<< "Increase - z-position: e" << endl;
	cout<< "Decrease - z-position: q" << endl;
	cout<< endl;
	cout<< " - Light -" << endl;
	cout<< "Increase - x-position: l" << endl;
	cout<< "Decrease - x-position: j" << endl;
	cout<< "Increase - y-position: i" << endl;
	cout<< "Decrease - y-position: k" << endl;
	cout<< "Increase - z-position: o" << endl;
	cout<< "Decrease - z-position: u" << endl;

}

void Renderer::handleInputEvents() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			SDL_Quit();
			exit(0);
		}
		if(event.type == SDL_KEYDOWN ){
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				exit(0);
				break;
			//play-speed
			case SDLK_PLUS:
				if (speed >= 50)
					speed -= 50;
				break;
			case SDLK_MINUS:
				speed += 50;
				break;
			//object
			case SDLK_w:
				objectPosition.y += 5;
				break;
			case SDLK_s:
				objectPosition.y -= 5;
				break;
			case SDLK_a:
				objectPosition.x -= 5;
				break;
			case SDLK_d:
				objectPosition.y += 5;
				break;
			case SDLK_q:
				objectPosition.z -= 5;
				break;
			case SDLK_e:
				objectPosition.z += 5;
				break;
			//light
			case SDLK_i:
				lightPosition[1] += 5;
				break;
			case SDLK_k:
				lightPosition[1] -= 5;
				break;
			case SDLK_j:
				lightPosition[0] -= 5;
				break;
			case SDLK_l:
				lightPosition[0] += 5;
				break;
			case SDLK_u:
				lightPosition[2] -= 5;
				break;
			case SDLK_o:
				lightPosition[2] += 5;
				break;
			default:
				break;
			}
		}
	}
}

void Renderer::setupPerspective(){
	if (!isPerspectiveSet){
		//setup Frustum
		//gluPerspective(45.0,videoSize.width/videoSize.height,0,100.0);
		//double zNear=1.5;
		//double zFar=10;
		//glFrustum( 0, videoSize.width, 0, videoSize.height, zNear, zFar );
		glOrtho(0, videoSize.width, 0, videoSize.height, -500, 500);
		isPerspectiveSet=true;
	}
}

void Renderer::setupObjectPostion(std::vector<cv::Mat>& video){
	cout << "Position object and light into the scene" << endl;

	showKeyInfo();

	double cameraposition[16];
	for (int j = 0; j < 4; j++) {
		for (int k = 0; k < 4; k++)
			if (j == k)
				cameraposition[j * 4 + k] = 1;
			else
				cameraposition[j * 4 + k] = 0;
	}

	setupPerspective();

	for (size_t i = 0; i < video.size(); i++) {

		handleInputEvents();	//handle SDL events
		SDL_Delay(speed);		//reduce speed

		//TODO remove test...............................................................................
		double pi = 3.14159;
		double deg2Rad = pi/180.0;
		double angleX = 20+(i++);
		double sx = sin(angleX * deg2Rad);
		double cx = cos(angleX * deg2Rad);
		cv::Mat test_t = (cv::Mat_<double>(4, 4) << 1.0, 0.0, 0.0, 200, 0.0, 1.0, 0.0, 50, 0.0, 0.0, 1.0, 1, 0.0, 0.0, 0.0, 1.0);
		cv::Mat test_r = (cv::Mat_<double>(4, 4) << 1, 0, 0, 0, 0, cx, -sx, 0, 0, sx, cx, 0, 0, 0, 0, 1);
		setCameraPose(cameraposition, test_t, test_r);

		renderScene(video[i], cameraposition);
	}
}

void Renderer::render(std::vector<cv::Mat>& video, std::vector<cv::Mat> rotation, std::vector<cv::Mat> translation) {
	//open VideoWriter for output-video
	openOutputFile();

	double cameraposition[16];
	for (int j = 0; j < 4; j++) {
		for (int k = 0; k < 4; k++)
			if (j == k)
				cameraposition[j * 4 + k] = 1;
			else
				cameraposition[j * 4 + k] = 0;
	}

	setupPerspective();

	for (size_t i = 0; i < video.size(); i++) {
		//handleInputEvents();	//handle SDL events
		//SDL_Delay(speed);		//reduce speed

		//calculate rotation-translation-matrix
		//setCameraPose(camerapostion, rotation[i], translation[i]);

		//TODO remove test...............................................................................
		double pi = 3.14159;
		double deg2Rad = pi/180.0;
		double angleX = 20+(i++);
		double sx = sin(angleX * deg2Rad);
		double cx = cos(angleX * deg2Rad);
		cv::Mat test_t = (cv::Mat_<double>(4, 4) << 1.0, 0.0, 0.0, 200, 0.0, 1.0, 0.0, 50, 0.0, 0.0, 1.0, 1, 0.0, 0.0, 0.0, 1.0);
		cv::Mat test_r = (cv::Mat_<double>(4, 4) << 1, 0, 0, 0, 0, cx, -sx, 0, 0, sx, cx, 0, 0, 0, 0, 1);
		//test_r.at<double>(0, 0) = 0.5;
		//test_r.at<double>(1, 0) = 0.2;
		setCameraPose(cameraposition, test_t, test_r);

		//for (int j = 0; j < 4; j++) {
		//	for (int k = 0; k < 4; k++)
		//		cout <</*camerapostion[j*4+k]*/test_t.at<double>(j, k) << " ";
		//	cout << endl;
		//}
		//............................................................................................

		renderAndWriteFrame(video[i], cameraposition);
		writeNextFrame(video[i]);
	}
}

void Renderer::renderAndWriteFrame(cv::Mat& background, double cameraposition[16]) {
	cv::Mat renderedImage(videoSize, CV_8UC3);
	cv::flip(background, background, 0); //flip image - OpenGL and OpenCV different imagedata-structure

	//render object
	renderScene(background, cameraposition);

	//getOutput
	glReadPixels(0, 0, videoSize.width, videoSize.height, GL_BGR, GL_UNSIGNED_BYTE, (uchar*) renderedImage.data); //grab OpenGL frame buffer and store it in OpenCV image
	cv::flip(renderedImage, renderedImage, 0); //flip image
	background = renderedImage;
}

void Renderer::renderScene(cv::Mat& background, double cameraposition[16]) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		glOrtho(0, videoSize.width, 0, videoSize.height, -500, 500); //set ortho view
		//gluPerspective(45.0,videoSize.width/videoSize.height,0,100.0);
		glMatrixMode(GL_MODELVIEW);

		//Draw background - (slow solution)
		glDrawPixels(videoSize.width, videoSize.height, GL_BGR, GL_UNSIGNED_BYTE, background.data);

		//restore previous projective matrix
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);


	glPushMatrix();
		//set camerapostion
		//glRotatef(i++, 1.0f, 1.0f, 0.2f); //TODO
		//glLoadMatrixd(cameraposition);

		//update light
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

		//draw 3D-Object
		glPushMatrix();
			glTranslated(objectPosition.x, objectPosition.y, objectPosition.z); //Position in the room

			glScalef(scale, scale, scale); //TODO
			glColor4f(0.75, 0.75, 0.75, 1.0);
			//glutSolidTeapot(1);

			glBegin(GL_QUADS);		// Draw A Quad
				glVertex3f(1.0f, 1.0f, -1.0f);
				glVertex3f(-1.0f, 1.0f, -1.0f);
				glVertex3f(-1.0f, 1.0f, 1.0f);
				glVertex3f(1.0f, 1.0f, 1.0f);

				glVertex3f(1.0f, -1.0f, 1.0f);
				glVertex3f(-1.0f, -1.0f, 1.0f);
				glVertex3f(-1.0f, -1.0f, -1.0f);
				glVertex3f(1.0f, -1.0f, -1.0f);

				glVertex3f(1.0f, 1.0f, 1.0f);
				glVertex3f(-1.0f, 1.0f, 1.0f);
				glVertex3f(-1.0f, -1.0f, 1.0f);
				glVertex3f(1.0f, -1.0f, 1.0f);

				glVertex3f(1.0f, -1.0f, -1.0f);
				glVertex3f(-1.0f, -1.0f, -1.0f);
				glVertex3f(-1.0f, 1.0f, -1.0f);
				glVertex3f(1.0f, 1.0f, -1.0f);

				glVertex3f(-1.0f, 1.0f, 1.0f);
				glVertex3f(-1.0f, 1.0f, -1.0f);
				glVertex3f(-1.0f, -1.0f, -1.0f);
				glVertex3f(-1.0f, -1.0f, 1.0f);

				glVertex3f(1.0f, 1.0f, -1.0f);
				glVertex3f(1.0f, 1.0f, 1.0f);
				glVertex3f(1.0f, -1.0f, 1.0f);
				glVertex3f(1.0f, -1.0f, -1.0f);
			glEnd();

			//object.renderObject();

		glPopMatrix();
	glPopMatrix();

	glFlush();
	//glutSwapBuffers();
	SDL_GL_SwapBuffers();
}

double* Renderer::setCameraPose(double modelViewMatrixRecoveredCamera[16], cv::Mat& rotation, cv::Mat& translation) {
	//construct modelview matrix from rotation and translation (reversed y and z axis from OpenCV to OpenGL)
	// Left  Up  Fwd  Translation
	//   R   R   R |  t
	//  -R  -R  -R | -t
	//  -R  -R  -R | -t
	//   0   0   0 |  1
	//first column
	modelViewMatrixRecoveredCamera[0] = rotation.at<double>(0, 0);
	modelViewMatrixRecoveredCamera[1] = -rotation.at<double>(1, 0);
	modelViewMatrixRecoveredCamera[2] = -rotation.at<double>(2, 0);
	modelViewMatrixRecoveredCamera[3] = 0.0;
	//second column
	modelViewMatrixRecoveredCamera[4] = rotation.at<double>(0, 1);
	modelViewMatrixRecoveredCamera[5] = -rotation.at<double>(1, 1);
	modelViewMatrixRecoveredCamera[6] = -rotation.at<double>(2, 1);
	modelViewMatrixRecoveredCamera[7] = 0.0;
	//third column
	modelViewMatrixRecoveredCamera[8] = rotation.at<double>(0, 2);
	modelViewMatrixRecoveredCamera[9] = -rotation.at<double>(1, 2);
	modelViewMatrixRecoveredCamera[10] = -rotation.at<double>(2, 2);
	modelViewMatrixRecoveredCamera[11] = 0.0;
	//fourth column
	modelViewMatrixRecoveredCamera[12] = translation.at<double>(0, 0);
	modelViewMatrixRecoveredCamera[13] = -translation.at<double>(1, 0);
	modelViewMatrixRecoveredCamera[14] = -translation.at<double>(2, 0);
	modelViewMatrixRecoveredCamera[15] = 1.0;

	return modelViewMatrixRecoveredCamera;
}

void Renderer::writeNextFrame(cv::Mat& frame) {
	if (videoWriter.isOpened()) {
		videoWriter.write(frame);
	} else {
		std::cout << "Error: Video not open" << std::endl;
	}
}
