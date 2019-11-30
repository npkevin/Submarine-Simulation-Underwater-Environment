#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <time.h>

#include <gl/glut.h>
#include <glm.hpp>

#include "QuadMesh.h"

#define DEG2RAD 3.14159f/180.0f

void initOpenGL(int, int);
void displayHandler(void);
void reshapeHandler(int, int);
void keyboardInputHandler(unsigned char, int, int);
void mouseButtonHandler(int, int, int, int);
void mouseMotionHandler(int, int);
void keyboardUp(unsigned char key, int x, int y);
void idle();
void readTexel();

void drawLittleBalckSubmarine();
void drawPropellor(int pos);
GLuint texture_id;

// keyDown flags
bool isDownW = false;
bool isDownS = false;
bool isDownA = false;
bool isDownD = false;
bool isDownSpace = false;
bool isDownC = false;

// Movement stuff
float subAltitude = 1.0;
float minAltitude = 1.0;
float backPropRotation = 0.0f;
float leftPropRotation = 0.0f;
float rightPropRotation = 0.0f;
float rise_decline_angle = 0.0f;
float max_rise_angle = 20.0f;
float min_decline_angle = -20.0f;
float submarineRotation = 0.0;
float speed = 0.01f;
float minSpeed = 0.002;
float maxSpeed = 0.05;
float xSub = 0.0f;
float zSub = 0.0f;

// Other
int mainWindowID;
int deltaTime = 0;
int prevTime = 0;
bool mmDown = false;
bool rmDown = false;
bool lmDown = false;

std::vector<Metaball> ballList;
int ballIndex = 0;
float ballHeight = 5;
float ballWidth = 0.1;

// Settings
int vWidth = 1000;
int vHeight = 800;

// Terrain
static QuadMesh terrain;
const int meshSize = 64; // meshSize x meshSize (quads)
const int meshWidth = 64;
const int meshLength = 64;

GLubyte* texel;

static GLfloat light_position[] = { 100.0F, 100.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

//static GLfloat surface_ambient[] = { 0.3F, 0.5F, 0.3F, 1.0F };
//static GLfloat surface_specular[] = { 0.1F, 0.35F, 0.1F, 0.5F };
//static GLfloat surface_diffuse[] = { 0.1F, 0.2F, 0.1F, 1.0F };


// DRONE MATERIALS
static GLfloat drone_mat_ambient[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.01F, 0.01F, 0.01F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.05F, 0.05F, 0.05F, 1.0F };
static GLfloat drone_mat_shininess[] = { 1.0F };
// Blue
GLfloat drone_blade_mat_ambient[] = { 0.1F, 0.2F, 0.3F, 1.0F };
GLfloat drone_blade_mat_specular[] = { 0.01F, 0.2F, 0.3F, 1.0F };
GLfloat drone_blade_mat_diffuse[] = { 0.05F, 0.1F, 0.3F, 1.0F };
GLfloat drone_blade_mat_shininess[] = { 1.0F };

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - vWidth) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - vHeight) / 2);
	mainWindowID = glutCreateWindow("A3 - 500627132|Kevin Nguyen, 500646804|Kevin Doung");
	initOpenGL(vWidth, vHeight);

	// Callbacks
	glutDisplayFunc(displayHandler);
	glutReshapeFunc(reshapeHandler);
	glutIdleFunc(idle);
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboardInputHandler);
	glutKeyboardUpFunc(keyboardUp);

	// Enter Main loop
	glutMainLoop();
	return 0;
	return 0;
}

void initOpenGL(int w, int h) {
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);


	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.7F, 0.7F, 0.7F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(0.0f, 0.0f, 0.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	terrain = NewQuadMesh(meshSize);

	glEnable(GL_TEXTURE_2D);
	readTexel();
	InitMeshQM(&terrain, meshSize, origin, meshWidth, meshLength, dir1v, dir2v);

	// Bloblist premade
	Metaball b1;
	Metaball b2;
	Metaball b3;
	Metaball b4;
	Metaball b5;
	Metaball b6;
	b1.width = 0.05;
	b1.height = 10;
	b1.pos = NewVector3D(41, 0, -11);
	b2.width = 0.05;
	b2.height = 10;
	b2.pos = NewVector3D(7, 0, -54);
	b3.width = 0.08;
	b3.height = 10;
	b3.pos = NewVector3D(19, 0, -28);
	b4.width = 0.06;
	b4.height = 7;
	b4.pos = NewVector3D(35, 0, -26);
	b5.width = 0.01;
	b5.height = -5;
	b5.pos = NewVector3D(3, 0, -20);
	b6.width = 0.1;
	b6.height = 20;
	b6.pos = NewVector3D(14, 0, -53);
	ballList.push_back(b1); ballList.push_back(b2);
	ballList.push_back(b3); ballList.push_back(b4);
	ballList.push_back(b5); ballList.push_back(b6);
	UpdateMesh(&terrain, ballList);

	
	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	//SetMaterialQM(&terrain, ambient, diffuse, specular, 0.2);
}

void reshapeHandler(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)w / h, 0.2, 300.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void displayHandler(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glMaterialfv(GL_FRONT, GL_AMBIENT, surface_ambient);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, surface_specular);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, surface_diffuse);

	// Draw ground mesh
	DrawMeshQM(&terrain, meshSize);
	
	
	// Set drone material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);
	
	glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
	glEnable(GL_TEXTURE_GEN_T);
	glPushMatrix();
		glTranslatef(xSub, subAltitude, zSub);
		glRotatef(submarineRotation, 0, 1, 0);
		drawLittleBalckSubmarine();
	glPopMatrix();
	glDisable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
	glDisable(GL_TEXTURE_GEN_T);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		xSub, subAltitude + 20, zSub + 20,
		xSub, subAltitude, zSub,
		0.0, 1.0, 0.0);

	glutSwapBuffers();
}
// state:0 == keyDown
void mouseButtonHandler(int button, int state, int x, int y) {
	
}

void mouseMotionHandler(int x, int y) {
	
}

void keyboardInputHandler(unsigned char key, int x, int y) {
	switch (key) {
		//esc
	case 27:
		glutDestroyWindow(mainWindowID);
		break;
	case '-':
		speed -= 0.002;
		if (speed < minSpeed) speed = minSpeed;
		printf("speed: %f\n", speed);
		break;
	case '=':
		speed += 0.002;
		if (speed > maxSpeed) speed = maxSpeed;
		printf("speed: %f\n", speed);
		break;
	case 'w':
		if (!isDownW) isDownW = true;

		break;
	case 's':
		if (!isDownS) isDownS = true;
		break;
	case 'a':
		if (!isDownA) isDownA = true;
		break;
	case 'd':
		if (!isDownD) isDownD = true;
		break;
	case ' ':
		if (!isDownSpace) isDownSpace = true;
		break;
	case 'c':
		if (!isDownC) isDownC = true;
		break;
	default:
		break;
	}
}

void keyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		if (isDownW) isDownW = false;
		break;
	case 's':
		if (isDownS) isDownS = false;
		break;
	case 'a':
		if (isDownA) isDownA = false;
		break;
	case 'd':
		if (isDownD) isDownD = false;
		break;
	case ' ':
		if (isDownSpace) isDownSpace = false;
		break;
	case 'c':
		if (isDownC) isDownC = false;
		break;
	default:
		break;
	}

}

void idle() {
	int now = clock();
	deltaTime = now - prevTime;
	prevTime = now;
	//printf("deltaTime: %d\n", deltaTime);

	if (isDownW || isDownS) {
		if (isDownW) {
			backPropRotation += deltaTime * speed * 50;
			leftPropRotation += deltaTime * speed * 50;
			rightPropRotation += deltaTime * speed * 50;
			xSub -= deltaTime * speed * sinf(submarineRotation * DEG2RAD);
			zSub -= deltaTime * speed * cosf(submarineRotation * DEG2RAD);
		}
		if (isDownS) {
			backPropRotation -= deltaTime * speed * 50;
			leftPropRotation -= deltaTime * speed * 50;
			rightPropRotation -= deltaTime * speed * 50;
			xSub += deltaTime * speed * sinf(submarineRotation * DEG2RAD);
			zSub += deltaTime * speed * cosf(submarineRotation * DEG2RAD);
		}
		glutPostRedisplay();
	}
	if (isDownA || isDownD) {
		if (isDownA) {
			if (submarineRotation > 360) submarineRotation = 0;
			submarineRotation += deltaTime * speed * 20;
			leftPropRotation -= deltaTime * speed * 50;
			rightPropRotation += deltaTime * speed * 50;
		}
		if (isDownD) {
			if (submarineRotation < 0) submarineRotation = 360;
			submarineRotation -= deltaTime * speed * 20;
			leftPropRotation += deltaTime * speed * 50;
			rightPropRotation -= deltaTime * speed * 50;
		}
		glutPostRedisplay();
	}
	if (isDownSpace || isDownC) {
		if (isDownSpace) {
			subAltitude += deltaTime * speed * fabs(rise_decline_angle / max_rise_angle);
			rise_decline_angle += deltaTime * 0.1;
			if (rise_decline_angle > max_rise_angle) rise_decline_angle = max_rise_angle;
		}
		if (isDownC) {
			subAltitude -= deltaTime * speed * fabs(rise_decline_angle / min_decline_angle);
			rise_decline_angle -= deltaTime * 0.1;
			if (subAltitude < minAltitude) subAltitude = minAltitude;
			if (rise_decline_angle < min_decline_angle) rise_decline_angle = min_decline_angle;
		}
		backPropRotation += deltaTime * 0.2;
		leftPropRotation += deltaTime * 0.2;
		rightPropRotation += deltaTime * 0.4;
		glutPostRedisplay();
	}
	else {
		if (rise_decline_angle != 0) {
			if (rise_decline_angle > 0) {
				rise_decline_angle -= deltaTime * 0.1;
				if (rise_decline_angle < 0) rise_decline_angle = 0;
			}
			if (rise_decline_angle < 0) {
				rise_decline_angle += deltaTime * 0.1;
				if (rise_decline_angle > 0) rise_decline_angle = 0;
			}
		}
		glutPostRedisplay();
	}

	if (backPropRotation > 360) backPropRotation = 0;
	if (backPropRotation < 0) backPropRotation = 360;
	if (leftPropRotation > 360) leftPropRotation = 0;
	if (leftPropRotation < 0) leftPropRotation = 360;
	if (rightPropRotation > 360) rightPropRotation = 0;
	if (rightPropRotation < 0) rightPropRotation = 360;
	
}

void drawLittleBalckSubmarine() {
	// Body, CTM = IMTR (copy)
	glPushMatrix();

	glRotatef(rise_decline_angle, 1, 0, 0);
	// CTM = IMTRT
	glTranslatef(0, 1, 0);
	// CTM = IMTRTS
	glScalef(1.0F, 1.0F, 2.0F);
	glutSolidSphere(1.0F, 16, 16);

	// CTM = IMTRTST
	glTranslatef(0, 0, -0.1);
	// Push matrix here
	drawPropellor(0);


	// Window
	// CTM = IMTRTS
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_blade_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_blade_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_blade_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_blade_mat_shininess);

	// CTM = IMTRTST
	glTranslatef(0, 0.4, -0.1);
	// CTM = IMTRTSTS
	glScalef(1, 1, 0.75);
	glutSolidSphere(0.8, 16, 16);

	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);
	glPopMatrix();
	// CTM = IMTRTS

	// Left and right wings/propellors
	glPushMatrix();
	// CTM = IMTRTSS
	glScalef(0.85, 0.85, 0.85);
	// CTM = IMTRTSST
	glTranslatef(0, 0, -1);
	// Left propellor
	glPushMatrix();
	// CTM = IMTRTSSTT
	glTranslatef(-1.6, 0, 0);
	drawPropellor(1);
	glPopMatrix();

	// Right propellor
	glPushMatrix();
	// CTM = IMTRTSSTT
	glTranslatef(1.6, 0, 0);
	drawPropellor(2);
	glPopMatrix();
	glPopMatrix();



	glPopMatrix();
}

void drawPropellor(int pos) {
	// Propellor
	// Copy CTM (CTM = IMTRTST)
	glPushMatrix();
	// Cone thing and back
	// Copy CTM (CTM = IMTRTST)
	glPushMatrix();
	//GLUquadric* propellorShieldBack;
	//propellorShieldBack = gluNewQuadric();
	// CTM = IMTRTSTT
	glTranslatef(0, 0, 1);
	//gluDisk(propellorShieldBack, 0, 0.8, 16, 16);
	// CTM = IMTRTSTTT
	glTranslatef(0, 0, 0.001);
	glutSolidCone(0.5, 0.5, 8, 8);
	glPopMatrix();

	// CTM = IMTRTST
	glTranslatef(0, 0, 1);
	GLUquadric* propellorShield;
	propellorShield = gluNewQuadric();
	gluCylinder(propellorShield, 0.8F, 0.8F, 0.5F, 16, 16);

	// Blades
	// Copy CTM (CTM = IMTRTST)
	glPushMatrix();

	// CTM = IMTRTSTT
	glTranslatef(0, 0, 0.3);
	float propellorRotation = 0.0;

	if (pos == 0) {
		propellorRotation = backPropRotation;
	}
	else if (pos == 1) {
		propellorRotation = leftPropRotation;
	}
	else if (pos == 2) {
		propellorRotation = rightPropRotation;
	}

	// CTM = IMTRTSTTR
	glRotatef(propellorRotation, 0, 0, -1);

	GLUquadric* blade1;
	GLUquadric* blade2;
	GLUquadric* blade3;
	GLUquadric* blade4;

	blade2 = gluNewQuadric();
	blade3 = gluNewQuadric();
	blade4 = gluNewQuadric();
	blade1 = gluNewQuadric();

	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_blade_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_blade_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_blade_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_blade_mat_shininess);

	//Blade 1
	glPushMatrix();
	// CTM = IMTRTSTTRR
	glRotatef(10, 1, 1, 0);
	gluPartialDisk(blade1, 0, 0.7f, 8, 8, 0, 80);
	glPopMatrix();
	//Blade 2
	glPushMatrix();
	// CTM = IMTRTSTTRR
	glRotatef(10, 1, -1, 0);
	gluPartialDisk(blade1, 0, 0.7f, 8, 8, 90, 80);
	glPopMatrix();
	//Blade 3
	glPushMatrix();
	// CTM = IMTRTSTTRR
	glRotatef(10, -1, -1, 0);
	gluPartialDisk(blade1, 0, 0.7f, 8, 8, 180, 80);
	glPopMatrix();
	//Blade 4
	glPushMatrix();
	// CTM = IMTRTSTTRR
	glRotatef(10, -1, 1, 0);
	gluPartialDisk(blade1, 0, 0.7f, 8, 8, 270, 80);
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);
	glPopMatrix();
	// Blades END
	glPopMatrix();
	// Propellor END
}

void readTexel() {

	GLuint texture_id;

	const char* filePath = "/Users/Kevin/Desktop/sand.bmp";
	unsigned char* texel;
	texel = (unsigned char*)malloc(2048 * 2048 * 3);
	FILE* f;
	fopen_s(&f, filePath, "rb");

	if (f == NULL) {
		printf("Failed to open file\n");
	}
	else {
		printf("Opened file!\n");
	}

	if (fread(texel, 2048 * 2048 * 3, 1, f)) {
		printf("Read success!\n");
	}

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, texel);
}

bool testCollision() {
	
	return false;
}