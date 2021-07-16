#include <GLFW/glfw3.h>
//#include <GL/glut.h>
#include <gl/GLU.h>
#include <gl/GL.h>
#include <math.h>
#include <sys/timeb.h>
#include <time.h>
using namespace std;

/* PI */
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

float rand_y1 = 0;
float rand_y2 = 2;
float rand_y3 = 0.5;
float rand_y4 = -1;
double missle_pos[9];
double plane2_vertex[5];
GLdouble missles[4][16]; // 4 missle model view matix
GLdouble plane1[16];
GLdouble plane2[16];
GLdouble project[16];
GLint viewport[4];

void Cube() 
{
	glBegin(GL_QUAD_STRIP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 1.0f, -1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glEnd();
	glBegin(GL_QUAD_STRIP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();
}

void Circle() 
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, 0.0f);
	int i = 0;
	for (i = 0; i <= 375; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
}

void circle_adj(float rad) 
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, 0.0f);
	int i = 0;
	for (i = 0; i <= 375; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(rad * sin(p), rad * cos(p), 0.0f);
	}
	glEnd();
}

void Cylinder() 
{
	glBegin(GL_QUAD_STRIP);
	int i = 0;
	for (i = 0; i <= 375; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(sin(p), cos(p), 1.0f);
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
	Circle();
	glTranslatef(0, 0, 1);
	Circle();
}

void cylinder_adj(float length, float rad) 
{
	glBegin(GL_QUAD_STRIP);
	int i = 0;
	for (i = 0; i <= 375; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(rad * sin(p), rad * cos(p), length);
		glVertex3f(rad * sin(p), rad * cos(p), 0.0f);
	}
	glEnd();
	circle_adj(rad);
	glTranslatef(0, 0, 1);
	circle_adj(rad);
}

void Cone() 
{
	glBegin(GL_QUAD_STRIP);
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(0, 0, 1.0f);
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
	Circle();
}

void cone_adj(float length, float rad)
{
	glBegin(GL_QUAD_STRIP);
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(0, 0, length);
		glVertex3f(rad * sin(p), rad * cos(p), 0.0f);
	}
	glEnd();
	//Circle();
	circle_adj(rad);
}



void AirPlane(float x, float y, float z)
{
	glPushMatrix();
	glTranslatef(x, y, z);
	//glRotatef(f, 1, 1, 1);
	glColor3f(0.5, 1.5, 0.5);
	glTranslatef(0, 0, -0.06);
	glRotatef((float)glfwGetTime() * 100.f, 0.f, 1.f, 0.f);
	//glTranslatef(-1, 0, 0);
	glTranslatef(0, 0, 0.5);
	glScalef(0.1, 0.05, 1);
	Cube(); 
	glPopMatrix();

	glTranslatef(0, -0.1, 0);
	glScalef(0.1, 0.1, 0.1);
	//glRotatef((float)glfwGetTime() * 100.f, 0.f, 1.f, 0.f);
	Cube();
	glScalef(10, 10, 10);

	glColor3f(1, 0, 1);
	glTranslatef(0.04, -0.05, -0.9);
	glScalef(0.1, 0.1, 1.5);
	Cylinder();
	glColor3f(0, 1, 0);
	glScalef(1, 1, 0.2);
	Cone();
	glColor3f(0, 1, 1);
	glTranslatef(0, 0.7, -4.5);
	glScalef(0.2, 2, 1);
	Cube();

	glTranslatef(-13, 0.3, 0);
	glScalef(27, 0.1, 1);
	Cube();

	glPopMatrix();
}

//Airplane 2
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;
void Airplane2()
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glScalef(0.015f, 0.015f, 0.015f);
	glRotatef(-20, 0.0f, 1.0f, 0.0f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	glPushMatrix();

	glRotatef(xRot, 1.0f, 0.0f, 0.0f);

	glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	glGetDoublev(GL_MODELVIEW_MATRIX, plane2);  // acquire model view matrix of the plane after all translation and rotation

	glColor3ub(0, 255, 0);

	glBegin(GL_TRIANGLES);

	glVertex3f(0.0f, 0.0f, 60.0f);
	glVertex3f(-15.0f, 0.0f, 30.0f);
	glVertex3f(15.0f, 0.0f, 30.0f);

	glVertex3f(15.0f, 0.0f, 30.0f);
	glVertex3f(0.0f, 15.0f, 30.0f);
	glVertex3f(0.0f, 0.0f, 60.0f);

	glVertex3f(0.0f, 0.0f, 60.0f);
	glVertex3f(0.0f, 15.0f, 30.0f);
	glVertex3f(-15.0f, 0.0f, 30.0f);

	glColor3ub(64, 64, 64);

	glVertex3f(-15.0f, 0.0f, 30.0f);
	glVertex3f(0.0f, 15.0f, 30.0f);
	glVertex3f(0.0f, 0.0f, -56.0f);

	glVertex3f(0.0f, 0.0f, -56.0f);
	glVertex3f(0.0f, 15.0f, 30.0f);
	glVertex3f(15.0f, 0.0f, 30.0f);

	glVertex3f(15.0f, 0.0f, 30.0f);
	glVertex3f(-15.0f, 0.0f, 30.0f);
	glVertex3f(0.0f, 0.0f, -56.0f);
	glColor3ub(192, 192, 192);


	glVertex3f(0.0f, 2.0f, 27.0f);
	glVertex3f(-60.0f, 2.0f, -8.0f);
	glVertex3f(60.0f, 2.0f, -8.0f);

	glVertex3f(60.0f, 2.0f, -8.0f);
	glVertex3f(0.0f, 7.0f, -8.0f);
	glVertex3f(0.0f, 2.0f, 27.0f);

	glVertex3f(60.0f, 2.0f, -8.0f);
	glVertex3f(-60.0f, 2.0f, -8.0f);
	glVertex3f(0.0f, 7.0f, -8.0f);

	glVertex3f(0.0f, 2.0f, 27.0f);
	glVertex3f(0.0f, 7.0f, -8.0f);
	glVertex3f(-60.0f, 2.0f, -8.0f);

	glColor3ub(255, 255, 0);
	glVertex3f(-30.0f, -0.50f, -57.0f);
	glVertex3f(30.0f, -0.50f, -57.0f);
	glVertex3f(0.0f, -0.50f, -40.0f);

	glVertex3f(0.0f, -0.5f, -40.0f);
	glVertex3f(30.0f, -0.5f, -57.0f);
	glVertex3f(0.0f, 4.0f, -57.0f);

	glVertex3f(0.0f, 4.0f, -57.0f);
	glVertex3f(-30.0f, -0.5f, -57.0f);
	glVertex3f(0.0f, -0.5f, -40.0f);

	glVertex3f(30.0f, -0.5f, -57.0f);
	glVertex3f(-30.0f, -0.5f, -57.0f);
	glVertex3f(0.0f, 4.0f, -57.0f);

	glColor3ub(255, 0, 0);
	glVertex3f(0.0f, 0.5f, -40.0f);
	glVertex3f(3.0f, 0.5f, -57.0f);
	glVertex3f(0.0f, 25.0f, -65.0f);

	glVertex3f(0.0f, 25.0f, -65.0f);
	glVertex3f(-3.0f, 0.5f, -57.0f);
	glVertex3f(0.0f, 0.5f, -40.0f);

	glVertex3f(3.0f, 0.5f, -57.0f);
	glVertex3f(-3.0f, 0.5f, -57.0f);
	glVertex3f(0.0f, 25.0f, -65.0f);
	glEnd();

	glPopMatrix();
}

void Missle()
{
	float cone_length = 0.6;
	float cylinder_length = 1;
	float rad = 0.15;
	glRotatef(90, 0.0, -1.0, 0.0);
	glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
	glColor3f(154.0 / 256.0, 205.0 / 256.0, 50.0 / 256.f);
	cone_adj(cone_length, rad);
	glTranslatef(0.0, 0.0, -cylinder_length);
	glColor3f(0.1328, 0.5429, 0.1328);
	cylinder_adj(cylinder_length, rad);

	float tail_length = 0.5;
	float tail_height = rad;
	float tail_width = 0.1;

	glColor3f(107.0 / 256.0, 142.0 / 256.0, 35.0 / 256.f);
	glColor3f(154.0 / 256.0, 205.0 / 256.0, 50.0 / 256.f);
	for (int i = 0; i < 4; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(rad * 0.8, 0, -cylinder_length + tail_length);
		glVertex3f(rad * 0.8, -tail_width, -cylinder_length);
		glVertex3f(rad * 0.8, tail_width, -cylinder_length);
		glVertex3f(rad + tail_height, 0, -cylinder_length * 1.1);
		glVertex3f(rad * 0.8, 0, -cylinder_length + tail_length);
		glVertex3f(rad * 0.8, -tail_width, -cylinder_length);
		glEnd();
		glRotatef(90, 0, 0, 1);
	}
}


/*generate the map consists of 4 missiles flying from right to left with random y position*/
void missile_map(double start_time) {
	// Camera is automatically at position (0,0,0)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	float x_move_range = 7;
	float y_move_range = 2;
	float speed = 2000;
	float cur_time = glfwGetTime();
	float dis = (int)(pow(0.25 * (cur_time - start_time), 2) * speed) % (int)(x_move_range * 2000) / 1000.0;	//loop from 0 to 2*x_move_range
	float dis2 = (int)(pow(0.25 * (cur_time - start_time), 2) * speed + x_move_range * 1000) % (int)(x_move_range * 2000) / 1000.0;
	float depth = -9.5;
	struct timeb timebuffer;
	ftime(&timebuffer);
	srand(timebuffer.time * 1000 + timebuffer.millitm);
	//srand((unsigned)time(NULL));

	//generate random y position
	if (2 * x_move_range - dis < 0.5) {
		//rand_y1 = (int)(rand() * 1111) % (int)(y_move_range * 2000) / 1000.0 - y_move_range;
		rand_y1 = (rand() % 4000) / 1000.0 - y_move_range;
		//rand_y2 = (int)(rand() * 1234) % (int)(y_move_range * 2000) / 1000.0 - y_move_range;
		rand_y2 = (rand() % 4000) / 1000.0 - y_move_range;
		while (abs(rand_y2 - rand_y1) < 1.0)
			rand_y2 = (rand() % 4000) / 1000.0 - y_move_range;
	}
	if (2 * x_move_range - dis2 < 0.5) {
		//rand_y3 = (int)(rand() * 1111) % (int)(y_move_range * 2000) / 1000.0 - y_move_range;
		rand_y3 = (rand() % 4000) / 1000.0 - y_move_range;
		//rand_y4 = (int)(rand() * 1234) % (int)(y_move_range * 2000) / 1000.0 - y_move_range;
		rand_y4 = (rand() % 4000) / 1000.0 - y_move_range;
		while (abs(rand_y4 - rand_y3) < 1.0)
			rand_y4 = (rand() % 4000) / 1000.0 - y_move_range;
	}

	glGetDoublev(GL_PROJECTION_MATRIX, project);
	glGetIntegerv(GL_VIEWPORT, viewport);

	//the first missile
	glPushMatrix();
	glPushMatrix();
	glTranslatef(x_move_range - dis, rand_y1, depth);
	glGetDoublev(GL_MODELVIEW_MATRIX, missles[0]);
	Missle();

	//the second missile
	glPopMatrix();
	glTranslatef(x_move_range - dis, rand_y2, depth);
	glGetDoublev(GL_MODELVIEW_MATRIX, missles[1]);
	Missle();

	//the third missile
	glPopMatrix();
	glPushMatrix();
	glTranslatef(x_move_range - dis2, rand_y3, depth);
	glGetDoublev(GL_MODELVIEW_MATRIX, missles[2]);
	Missle();

	//the forth missile
	glPopMatrix();
	glPushMatrix();
	glTranslatef(x_move_range - dis2, rand_y4, depth);
	glGetDoublev(GL_MODELVIEW_MATRIX, missles[3]);
	Missle();


	missle_pos[0] = x_move_range - dis;
	missle_pos[1] = rand_y1;
	missle_pos[2] = x_move_range - dis;
	missle_pos[3] = rand_y2;
	missle_pos[4] = x_move_range - dis2;
	missle_pos[5] = rand_y3;
	missle_pos[6] = x_move_range - dis2;
	missle_pos[7] = rand_y4;
	missle_pos[8] = depth;

	glPopMatrix();
}

void drawSphere(double r, int lats, int longs) {
	int i, j;
	for (i = 0; i <= lats; i++) {
		double lat0 = M_PI * (-0.5 + (double)(i - 1) / lats);
		double z0 = r * sin(lat0);
		double zr0 = r * cos(lat0);

		double lat1 = M_PI * (-0.5 + (double)i / lats);
		double z1 = r * sin(lat1);
		double zr1 = r * cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= longs; j++) {
			double lng = 2 * M_PI * (double)(j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}
}


void drawCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{

	// draw the upper part of the cone
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, height);
	for (int angle = 0; angle < 360; angle++) {
		glVertex3f(sin((double)angle) * base, cos((double)angle) * base, 0.f);
	}
	glEnd();

	// draw the base of the cone
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 0);
	for (int angle = 0; angle < 360; angle++) {
		// normal is just pointing down
		glNormal3f(0, -1, 0);
		glVertex3f(sin((double)angle) * base, cos((double)angle) * base, 0.f);
	}
	glEnd();
}