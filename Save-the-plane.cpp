using namespace  std;
//#define GLEW_STATIC
#include <GL/glew.h> // Before any gl headers
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdlib.h>
#include <time.h>

#include "DrawPrimitives.h"
#include "MarkerTracker.h"
#include "PoseEstimation.h"

using namespace cv;

const int camera_width = 640;
const int camera_height = 480;
const int virtual_camera_angle = 30;
unsigned char bkgnd[camera_width * camera_height * 3];
float resultMatrix_0272[16];
float resultMatrix_1C44[16];
float crash_time = 0;
int code = 0;

float resultTransposedMatrix[16];


cv::VideoCapture cap(0);

time_t s_start;   // time the game starts
time_t s_end;     // current time

time_t e_start;   // time the game ends
time_t e_end;     // time the game has already ended

/* Initialization function  1 */
void initVideoStream(cv::VideoCapture& cap) {
	if (cap.isOpened()) {
		cap.release();
	}

	cap.open(0); // open the default camera
	if (!cap.isOpened()) {
		std::cout << "No webcam found, using a video file" << std::endl;
		cap.open("/home/berkay/MEGAsync/TUM/Augmented Reality/MarkerMovie.MP4");
		if (!cap.isOpened()) {
			std::cout << "No video file found. Exiting." << std::endl;
			exit(0);
		}
	}
}

/* Initialization function 2 */
void init(int argc, char* argv[]) {
	//pixel storage/packing stuff
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelZoom(1, -1);

	glEnable(GL_COLOR_MATERIAL); //keep the original color
	// Set background color, default is black
	glClearColor(0.0, 1.0, 1.0, 1.0);

	// Enable and set depth parameters -> Buffer saves the elements sorted by their depth (by the use of the z coordinate)
	glEnable(GL_DEPTH_TEST);
	// Set the value back to 1 when there was a swap
	glClearDepth(1);

	/*
	// Define lightening
	GLfloat light_pos[] = { 0, 0, 10, 0.0 }; // RGB-D
	GLfloat light_amb[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_dif[] = { 0.3, 0.3, 0.3, 1.0 };
	//GLfloat light_spe[] = { 1, 1, 1, 1.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_amb); 
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_dif); 
	//glLightfv(GL_LIGHT1, GL_SPECULAR, light_spe);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	*/
}

/* Set the background video */
void background(GLFWwindow* window, const cv::Mat& img_bgr) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	memcpy(bkgnd, img_bgr.data, sizeof(bkgnd)); //copy the camera
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, camera_width, 0.0, camera_height);
	glRasterPos2i(0, camera_height - 1);
	glDrawPixels(camera_width, camera_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, bkgnd);  //draw the backgound on the screen
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

/* Display a timer on the up left corner*/
int timer(const cv::Mat& img_bgr) {
	int time_taken = double(s_end - s_start - 5);
	string str = to_string(time_taken);
	str = "Time: " + str + "s";
	cv::putText(img_bgr, str, cv::Point(50, 50), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 255, 0), 2, false);
	return time_taken;
}

/* Set the airplane and the map*/
void display(GLFWwindow* window, const cv::Mat& img_bgr, std::vector<Marker>& markers) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up background video
	background(window, img_bgr);
	// Calculate Modelview (the transformation from world coordinate system to camera coordinates) -> Camera object
	glMatrixMode(GL_MODELVIEW);
	// Move to origin to avoid a null matrix
	glLoadIdentity();
	// Display the map
	//cout << "s_start" << s_start << endl;
	missile_map(crash_time);

	// Read all detected codes
	for (int i = 0; i < markers.size(); i++) {
		code = markers[i].code;
		if (code == 0x1C44) {		//jet
			for (int j = 0; j < 16; j++)
				resultMatrix_1C44[j] = markers[i].resultMatrix[j];
		}
		else if (code == 0x0272) { //heli
			for (int j = 0; j < 16; j++)
				resultMatrix_0272[j] = markers[i].resultMatrix[j];
		}
	}

	// draw airplane 1
	if (code == 0x0272) {
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				resultTransposedMatrix[x * 4 + y] = resultMatrix_0272[y * 4 + x];
			}
		}
		// Camera is automatically at position (0, 0, 0)
		glLoadMatrixf(resultTransposedMatrix);
		glRotatef(-90, 1, 90, 0);
		glScalef(0.03, 0.03, 0.03);
		glPushMatrix();
		AirPlane(0, 0, 0);
		//Airplane2();
	}

	//draw airplane 2
	else if (code == 0x1C44) {
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				resultTransposedMatrix[x * 4 + y] = resultMatrix_1C44[y * 4 + x];
			}
		}
		// Camera is automatically at position (0,0,0)
		glLoadMatrixf(resultTransposedMatrix);
		glRotatef(-90, 1, 90, 0);
		glScalef(0.03, 0.03, 0.03);
		glPushMatrix(); 
		//AirPlane(0, 0, 0);
		Airplane2();
	}

	else {}
}

/* Function for the screen reshape */
void reshape(GLFWwindow* window, int width, int height) {
	// Set a whole-window viewport
	// glViewport(x,y,width,height); (x,y) is the lower left corner of the window
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); //GLsizei: A non-negative binary integer, for sizes.

	glPixelZoom(width / 640, -height / 480);
	// Create a perspective projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float ratio = (GLfloat)width / (GLfloat)height;
	//int fov = 30;
	float near = 0.01f, far = 100.f;
	auto top = static_cast<float>(tan(virtual_camera_angle * M_PI / 360.0f) * near);
	float bottom = -top;
	float left = ratio * bottom;
	float right = ratio * top;
	glFrustum(left, right, bottom, top, near, far);
}

/* Before the game starts, wait for the code 0x1228 */
int wait(GLFWwindow* window, const cv::Mat& img_bgr, std::vector<Marker>& markers) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//set up background video
	background(window, img_bgr);
	// Calculate Modelview (the transformation from world coordinate system to camera coordinates) -> Camera object
	glMatrixMode(GL_MODELVIEW);
	// Move to origin to avoid a null matrix
	glLoadIdentity();

	int s_code = 0;	//start code

	cv::putText(img_bgr, "Show your marker ", cv::Point(150, 200), cv::FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	cv::putText(img_bgr, "to get started!", cv::Point(175, 250), cv::FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);

	// if start code 0x1288 is detected, then return 1
	for (int i = 0; i < markers.size(); i++) {
		s_code = markers[i].code;
		if (s_code == 0x1228) {
			return 1;
		}
	}
	return 0;
}

/* Before the game starts and after the game ends, only shows the backgound */
void only_background(GLFWwindow* window, const cv::Mat& img_bgr) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//set up background video
	background(window, img_bgr);
	// Calculate Modelview (the transformation from world coordinate system to camera coordinates) -> Camera object
	glMatrixMode(GL_MODELVIEW);
	// Move to origin to avoid a null matrix
	glLoadIdentity();

}

/* Put text on the screen before and after the game, output gaming time in other cases */
void textandtimer(const cv::Mat& img_bgr, int time_taken, string s_str, string e_str, int a_end) {

	// before game starts
	if (time_taken == 1) {
		cv::putText(img_bgr, "3", cv::Point(300, 250), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}
	else if (time_taken == 2) {
		cv::putText(img_bgr, "2", cv::Point(300, 250), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}
	else if (time_taken == 3) {
		cv::putText(img_bgr, "1", cv::Point(300, 250), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}
	else if (time_taken == 4) {
		cv::putText(img_bgr, s_str, cv::Point(100, 250), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}

	// after game ends
	if (a_end == 1)
	{
		cv::putText(img_bgr, e_str, cv::Point(100, 250), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
		string game_time = "Game time:" + to_string(time_taken - 5) + "s";
		cv::putText(img_bgr, "Game time: " + to_string(time_taken - 5) + "s", cv::Point(175, 300), cv::FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
		cv::putText(img_bgr, "Play again? " + to_string(10 - (e_end - e_start)) + "s", cv::Point(175, 350), cv::FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	}

	// in other cases
	if (a_end == 0 && time_taken >= 5) {
		timer(img_bgr);
	}
}

//collision test
int collision(const cv::Mat& img_bgr, std::vector<Marker>& markers, double win_height, double win_width, int time_taken) {

	//project missle from model coordinate to screen coordinate
	double winx[8] = { 0.0 }, winy[8] = { 0.0 }, winz;  //top-left and bottom-right points for 4 missles in screen coordinate
	double depth = missle_pos[8];
	for (int i = 0; i < 4; i++) {
		gluProject(missle_pos[2 * i] - 1.0, missle_pos[2 * i + 1] + 0.15, depth, missles[i], project, viewport, &winx[2 * i], &winy[2 * i], &winz);  // top-left
		gluProject(missle_pos[2 * i] + 1.0, missle_pos[2 * i + 1] - 0.15, depth, missles[i], project, viewport, &winx[2 * i + 1], &winy[2 * i + 1], &winz);  //bottom-right
		winy[2 * i] = (double)win_height - winy[2 * i];
		winy[2 * i + 1] = (double)win_height - winy[2 * i + 1];
	}

	//display keypoints used for collisiion test
	cv::namedWindow("collision test keypoints", CV_WINDOW_AUTOSIZE);
	for (int i = 0; i < 8; i++) {
		cv::Point2f p(winx[i], winy[i]);
		cv::circle(img_bgr, p, 10, cv::Scalar(255, 0, 0), 5);
	}

	//project plane2
	double plane2_winx[4], plane2_winy[4];
	gluProject(resultMatrix_1C44[3], resultMatrix_1C44[7], resultMatrix_1C44[11] + 60.f, plane2, project, viewport, &plane2_winx[0], &plane2_winy[0], &winz); //head
	gluProject(resultMatrix_1C44[3] + 60.f, resultMatrix_1C44[7] + 2.f, resultMatrix_1C44[11] - 8.f, plane2, project, viewport, &plane2_winx[1], &plane2_winy[1], &winz);  //right wing
	gluProject(resultMatrix_1C44[3] - 60.f, resultMatrix_1C44[7] + 2.f, resultMatrix_1C44[11] - 8.f, plane2, project, viewport, &plane2_winx[2], &plane2_winy[2], &winz);  //left wing
	gluProject(resultMatrix_1C44[3], resultMatrix_1C44[7] + 25.f, resultMatrix_1C44[11] - 65.f, plane2, project, viewport, &plane2_winx[3], &plane2_winy[3], &winz);  //tail

	double xmax = 0, xmin = 9999, ymax = 0, ymin = 9999;
	for (int i = 0; i < 4; i++) {
		float x = plane2_winx[i];
		float y = plane2_winy[i];
		cv::Point2f p1(x, (double)win_height - y);
		cv::circle(img_bgr, p1, 10, cv::Scalar(0, 255, 0), 5);

		// pick the max and min value on x and y direction from above 4 points
		if (x > xmax)
			xmax = x;
		else if (x < xmin)
			xmin = x;

		if (y > ymax)
			ymax = y;
		else if (y < ymin)
			ymin = y;
	}
	// approximate the plane to a rectangle
	double temp = ymax;
	ymax = win_height - ymin;
	ymin = win_height - temp;
	cv::Point2f plane2_esti1(xmin, ymin);  // top-left point
	cv::Point2f plane2_esti2(xmax, ymax);  //bottom-right point
	cv::circle(img_bgr, plane2_esti1, 10, cv::Scalar(0, 0, 255), 5);
	cv::circle(img_bgr, plane2_esti2, 10, cv::Scalar(0, 0, 255), 5);
	cv::Rect plane1_rect(xmin, win_height - ymax, xmax - xmin, ymax - ymin);
	//cv::rectangle(img_bgr, plane1_rect, cv::Scalar(0, 255, 0), 1, 8, 0);
	//cv::rectangle(img_bgr, plane2_esti1, plane2_esti2, cv::Scalar(0, 255, 0), 3, 8, 0);

	/*
	cv::Point2f p1(plane2_winx[0], (double)win_height - plane2_winy[0]);
	cv::circle(img_bgr, p1, 10, cv::Scalar(0, 255, 0), 5);
	cv::Point2f p2(plane2_winx[1], (double)win_height - plane2_winy[1]);
	cv::circle(img_bgr, p2, 10, cv::Scalar(0, 0, 255), 2);
	cv::Point2f p3(plane2_winx[3], (double)win_height - plane2_winy[3]);
	cv::circle(img_bgr, p3, 10, cv::Scalar(0, 0, 255), 2);
	*/

	// AABB collision test
	if (time_taken > 3) {
		for (int i = 0; i < 4; i++) {
			if (xmax > winx[2 * i] && xmin < winx[2 * i + 1]) {
				//printf("i: %d, xmax: %f, winx: %f, xmin: %f, winx2i+1: %f\n", i, xmax, winx[2 * i], xmin, winx[2 * i + 1]);
				if (ymin < winy[2 * i] && ymax>winy[2 * i + 1]) {
					cout << "crash" << endl;
					crash_time = glfwGetTime();
					return 1;
				}
			}
		}
	}

	cv::imshow("collision test keypoints", img_bgr);
	return 0;
}


/* Play again if showing the marker 0x0B44 */
int play_again(const cv::Mat& img_bgr, std::vector<Marker>& markers) {
	int e_code = 0;

	for (int i = 0; i < markers.size(); i++) {
		e_code = markers[i].code;
		if (e_code == 0x0B44) {
			return 1;
		}
	}
	return 0;
}



int main(int argc, char* argv[]) {
	int pa_flag = 1; //flag if play again

	GLFWwindow* window;
	//Initialize the library
	if (!glfwInit())
		return -1;

	//Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(640, 480, "Exercise 06", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* The big loop */
	while (pa_flag == 1) {
		//Set callback functions for GLFW
		glfwSetFramebufferSizeCallback(window, reshape);

		//Define where the rendering-thread should render the GLFW context
		glfwMakeContextCurrent(window);

		// The minimum number of screen updates to wait for until the buffers are swapped by glfwSwapBuffers
		glfwSwapInterval(1);

		// Initialize the frustum with the size of the framebuffer
		int window_width, window_height;
		glfwGetFramebufferSize(window, &window_width, &window_height);
		reshape(window, window_width, window_height);

		glViewport(0, 0, window_width, window_height);

		// Initialize the GL library
		// -> Give app arguments for configuration, e.g. depth or color should be activated or not
		init(argc, argv);

		//setup OpenCV
		cv::Mat img_bgr;
		initVideoStream(cap);
		double kMarkerSize = 0.02;
		MarkerTracker markerTracker(kMarkerSize); // initialize the marker
		std::vector<Marker> markers;

		float ResultMatrix[16];
		string s_str = "Game Start!";
		string e_str = "Game Over!";
		int s_flag = 0;  //flag if starts
		int a_start = 0;  //flag if already started
		int e_flag = 0;  //flag if ends
		int a_end = 0;   //flag if already ended

		int time_taken = 0;   //time after started

		/* The small loop */
		//Loop until the user closes the window
		while (!glfwWindowShouldClose(window))
		{
			//captue
			cap >> img_bgr;

			// track a marker
			markerTracker.findMarker(img_bgr, markers);

			// If not started yet, check if the marker 0x1228 is there
			if (a_start == 0) {
				s_flag = wait(window, img_bgr, markers);  //test if the code is there
			}
			// If not started yet, and the marker 0x1228 is tracked, starts the game
			if (s_flag == 1 && a_start == 0) {
				time(&s_start); // set the game starting time
				a_start = 1;  // set the flag for already started to 1
			}

			// get the current time and calculate the time after the game starts
			if (a_start == 1 && a_end != 1) {
				time(&s_end);
				time_taken = int(s_end - s_start);
			}

			// if already starts, set the output text and the timer
			if (time_taken != 0) {
				textandtimer(img_bgr, time_taken, s_str, e_str, a_end);
			}

			// set the rendering			
			if (time_taken == 0 || a_end == 1) {
				only_background(window, img_bgr);    // only shows the background before and after the game, 
			}
			else {
				display(window, img_bgr, markers);   //shows background, airplane and the map dduring the game
			}

			// if the game has already started and not ended yet, always test the collision
			if (a_start == 1 && a_end == 0) {
				e_flag = collision(img_bgr, markers, window_height, window_width, time_taken);	// e_flag = 1 if there is a collision
			}
			// if a collision is tested and the game has not ended yet, end the game
			if (e_flag == 1 && a_end == 0) {
				time(&e_start); // set the time when the game ends
				a_end = 1;   // set the flag for already ended to 1
				pa_flag = 0;  // set the flag for play again to 0
			}
			// if the game has already ended, calculate the time since the game ends
			if (a_start == 1 && a_end == 1) {
				time(&e_end);
				if ((e_end - e_start) < 11) {   // if within 10 seconds
					pa_flag = play_again(img_bgr, markers);   // check if the marker 0x0B44 is there, if so, play again (break the small loop and go to the big loop again)
					if (pa_flag == 1) {
						break;
					}
				}
				else if ((e_end - e_start) == 11) {			// ends the whole game (the big loop) if the marker is not there after 10 seconds
					pa_flag = 0;
					break;
				}
			}


			// Swap front and back buffers
			glfwSwapBuffers(window);

			//Poll for the process events
			glfwPollEvents();
		}
	}

	//int key = cvWaitKey(10);
	//if (key == 27) exit(0);

	glfwTerminate();
	return 0;
}