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

int frame = 0;
bool play=true;

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cout << "USAGE: argv[0]=InputVideo-FILENAME [, arg[1]=ObjectPositionData-Filename]" << std::endl;
		return EXIT_SUCCESS;
	}

	string video_filename = std::string(argv[1]);
	std::string* data_filename = 0;
	if (argc >=3)
		data_filename = new std::string(argv[2]);

	vector<Mat> frames;
	VideoCapture cap(video_filename);
	for (double i = 0; i < cap.get(CV_CAP_PROP_FRAME_COUNT); i++) {

		Mat frame;
		cap >> frame;
		frames.push_back(frame.clone());
	}

	cv::Mat K = (cv::Mat_<double>(3,4) <<  50, 0.0, 10.0, 0.0, 0.0, 50.0, 10.0, 0, 0.0, 0.0, -1.0 , 0);
	Renderer renderer(video_filename + "_render2.avi", cap, data_filename, K);

	renderer.setupObjectPostion(frames, /*translations*/frames, /*rotations*/frames);
	renderer.render(frames, /*translations*/frames, /*rotations*/frames);
}

Renderer::Renderer(std::string filename2, cv::Size size2, double framerate2, std::string* initFilename, cv::Mat K) {
	cout << filename2 << endl;
	init(filename2, initFilename, size2, framerate2, K);
}

Renderer::Renderer(std::string filename, cv::VideoCapture inputVideo, std::string* initFilename, cv::Mat K) {
	cout << filename << endl;
	Size s = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
	double framerate = inputVideo.get(CV_CAP_PROP_FPS);

	init(filename, initFilename, s, framerate, K);
}

Renderer::~Renderer() {
}

void Renderer::init(std::string filename2, std::string* initFilename, cv::Size size2, double framerate2, cv::Mat K_) {
	objectPositionFilename = initFilename;
	readObjectAndLightData();

	videoSize = size2;
	outputVideoFilename = filename2;
	framerate = framerate2;
	format = CV_FOURCC('D', 'I', 'V', 'X');
	K=K_;
	//object = QuadObject(scale);
	isPerspectiveSet=false;

	//init SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cout << "Video initialization failed: " << SDL_GetError() << endl;
		return;
	}
	/*const SDL_VideoInfo* videoInfo =*/ SDL_GetVideoInfo();
	if (SDL_SetVideoMode(videoSize.width, videoSize.height, 32, SDL_OPENGL ) == NULL) { //create window
		cerr << "Error creating SDL window!" << endl;
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_WM_SetCaption("MatchMover", NULL);
	//Antialiasing (SDL multisampling)
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); //16x


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

void Renderer::readObjectAndLightData() {
	objectPosition = Point3d(0, 0, -3);
	objectRotation[0]=0;
	objectRotation[1]=0;
	objectRotation[2]=0;
	scale = 1;

	lightPosition[0] = 0.0;
	lightPosition[1] = 0.0;
	lightPosition[2] = 0.0;

	if (objectPositionFilename != NULL){
		try {
			FileStorage fs(*objectPositionFilename, FileStorage::READ);
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
}

bool Renderer::openOutputFile() {
	videoWriter.open(outputVideoFilename, format, framerate, videoSize, true);
	if (videoWriter.isOpened())
		return true;
	else
		return false;
}

void Renderer::showKeyInfo(){
	cout<< "Keys:" << endl;
	cout<< " - Frame -" << endl;
	cout<< "Change frame: + & -" << endl;
	cout<< endl;
	cout<< " - Object -" << endl;
	cout<< "Increase - x-position: d" << endl;
	cout<< "Decrease - x-position: a" << endl;
	cout<< "Increase - y-position: w" << endl;
	cout<< "Decrease - y-position: s" << endl;
	cout<< "Increase - z-position: e" << endl;
	cout<< "Decrease - z-position: q" << endl;
	cout<< "Rotate - x-axis: y & x" << endl;
	cout<< "Rotate - y-axis: c & v" << endl;
	cout<< "Rotate - z-axis: b & n" << endl;
	cout<< endl;
	cout<< " - Light -" << endl;
	cout<< "Increase - x-position: l" << endl;
	cout<< "Decrease - x-position: j" << endl;
	cout<< "Increase - y-position: i" << endl;
	cout<< "Decrease - y-position: k" << endl;
	cout<< "Increase - z-position: o" << endl;
	cout<< "Decrease - z-position: u" << endl;
	cout<< endl;
	cout<< "Finish positioning: Enter" << endl;
}

void Renderer::handleInputEvents() {
	SDL_Event event;

	SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT) {
		SDL_Quit();
		exit(0);
	}
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE:
			exit(0);
			break;
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			play = false;
			break;
			//change frame
		case SDLK_PLUS:
			frame++;
			break;
		case SDLK_MINUS:
			if (frame > 0)
				frame--;
			break;
			//object
		case SDLK_w:
			objectPosition.y += 1;
			break;
		case SDLK_s:
			objectPosition.y -= 1;
			break;
		case SDLK_a:
			objectPosition.x -= 1;
			break;
		case SDLK_d:
			objectPosition.x += 1;
			break;
		case SDLK_q:
			objectPosition.z += 1;
			break;
		case SDLK_e:
			objectPosition.z -= 1;
			break;
		case SDLK_y:
			objectRotation[0] -= 5;
			break;
		case SDLK_x:
			objectRotation[0] += 5;
			break;
		case SDLK_c:
			objectRotation[1] -= 5;
			break;
		case SDLK_v:
			objectRotation[1] += 5;
			break;
		case SDLK_b:
			objectRotation[2] -= 5;
			break;
		case SDLK_n:
			objectRotation[2] += 5;
			break;
			//light
		case SDLK_i:
			lightPosition[1] += 1;
			break;
		case SDLK_k:
			lightPosition[1] -= 1;
			break;
		case SDLK_j:
			lightPosition[0] -= 1;
			break;
		case SDLK_l:
			lightPosition[0] += 1;
			break;
		case SDLK_u:
			lightPosition[2] -= 1;
			break;
		case SDLK_o:
			lightPosition[2] += 1;
			break;
		default:
			break;
		}
	}
}

void Renderer::setupPerspective(){
	if (!isPerspectiveSet){
		//setup Frustum
		glLoadIdentity();
		double zNear=0.1;
		double zFar=1000;
		//focal length
		double fx = K.at<double>(0, 0);
		double fy = K.at<double>(1, 1);
		//principal point
		//double cx = K.at<double>(0, 2);
		//double cy = K.at<double>(1, 2);

		double aspectRatio = (videoSize.width / videoSize.height) * (fy / fx);
		double fovy = 2 * atan(videoSize.height / (2.0 * fy)); 	//radiant
		double frustumHeight = tan(fovy / 2.0) * zNear;
		double frustumWidth = aspectRatio * frustumHeight;

		//gluPerspective(45.0,videoSize.width/videoSize.height,0.1,1000.0);
		//gluPerspective(fovy, aspectRatio, zNear, zFar);
		glFrustum(-frustumWidth , frustumWidth , -frustumHeight, frustumHeight, zNear,	zFar);
		//glFrustum(-zNear*cx/fx, zNear*(videoSize.width-cx)/fx, -zNear*cy/fy, zNear*(videoSize.height-cy)/fy, zNear, zFar);
		//glFrustum( -videoSize.width/2, videoSize.width, -videoSize.height/2, videoSize.height/2, zNear, zFar );
		isPerspectiveSet=true;

	}
}

void Renderer::setupObjectPostion(std::vector<cv::Mat>& video, std::vector<cv::Mat> rotation, std::vector<cv::Mat> translation){
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

	cout << "video-size: "<<video.size() << endl;
	while(play) {
		//calculate rotation-translation-matrix
		//setCameraPose(cameraposition, rotation[frame], translation[frame]);
		renderScene(video[frame], cameraposition);

		handleInputEvents();	//handle SDL events
		SDL_Delay(10);
	}
}

void Renderer::render(std::vector<cv::Mat>& video, std::vector<cv::Mat> rotation, std::vector<cv::Mat> translation) {
	//open VideoWriter for output-video
	cout << "Output-video" << endl;
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
		SDL_Delay(10);

		//calculate rotation-translation-matrix
		//setCameraPose(cameraposition, rotation[i], translation[i]);

		//TODO remove test...............................................................................
		double pi = 3.14159;
		double deg2Rad = pi/180.0;
		double angleX = 20;
		double sx = sin(angleX * deg2Rad);
		double cx = cos(angleX * deg2Rad);
		cv::Mat test_t = (cv::Mat_<double>(4, 4) << 1.0, 0.0, 0.0, 0, 0.0, 1.0, 0.0, 0, 0.0, 0.0, 1.0, 1, 0.0, 0.0, 0.0, 1.0);
		cv::Mat test_r = (cv::Mat_<double>(4, 4) << 1, 0, 0, 0, 0, cx, -sx, 0, 0, sx, cx, 0, 0, 0, 0, 1);
		//test_r.at<double>(0, 0) = 0.5;
		//test_r.at<double>(1, 0) = 0.2;
		setCameraPosition(cameraposition, test_t, test_r);

		renderAndWriteFrame(video[i], cameraposition);
		writeNextFrame(video[i]);
	}
}

void Renderer::renderAndWriteFrame(cv::Mat& background, double cameraposition[16]) {
	cv::Mat renderedImage(videoSize, CV_8UC3);
	cv::flip(background, background, 0); 	//flip image - OpenGL and OpenCV different imagedata-structure

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
		glOrtho(0, videoSize.width, 0, videoSize.height, 0, 10); //set ortho view
		glMatrixMode(GL_MODELVIEW);

		//Draw background - (slow solution)
		glDrawPixels(videoSize.width, videoSize.height, GL_BGR, GL_UNSIGNED_BYTE, background.data);

		//restore previous projective matrix
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);


	glPushMatrix();
		//set camerapostion
		//glLoadMatrixd(cameraposition);

		//update light
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

		//draw 3D-Object
		glPushMatrix();
			glTranslated(objectPosition.x, objectPosition.y, objectPosition.z); //Position in the room
			glRotated(objectRotation[0], 1, 0, 0);
			glRotated(objectRotation[1], 0, 1, 0);
			glRotated(objectRotation[2], 0, 0, 1);

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

double* Renderer::setCameraPosition(double modelViewCamera[16], cv::Mat& rotation, cv::Mat& translation) {
	//y and z axis are in OpenCV to OpenGL reversed
	//first column
	modelViewCamera[0] = rotation.at<double>(0, 0);
	modelViewCamera[1] = -rotation.at<double>(1, 0);
	modelViewCamera[2] = -rotation.at<double>(2, 0);
	modelViewCamera[3] = 0.0;
	//second column
	modelViewCamera[4] = rotation.at<double>(0, 1);
	modelViewCamera[5] = -rotation.at<double>(1, 1);
	modelViewCamera[6] = -rotation.at<double>(2, 1);
	modelViewCamera[7] = 0.0;
	//third column
	modelViewCamera[8] = rotation.at<double>(0, 2);
	modelViewCamera[9] = -rotation.at<double>(1, 2);
	modelViewCamera[10] = -rotation.at<double>(2, 2);
	modelViewCamera[11] = 0.0;
	//fourth column
	modelViewCamera[12] = translation.at<double>(0, 0);
	modelViewCamera[13] = -translation.at<double>(1, 0);
	modelViewCamera[14] = -translation.at<double>(2, 0);
	modelViewCamera[15] = 1.0;

	return modelViewCamera;
}

void Renderer::writeNextFrame(cv::Mat& frame) {
	if (videoWriter.isOpened()) {
		videoWriter.write(frame);
	} else {
		std::cout << "Error: Video not open" << std::endl;
	}
}
