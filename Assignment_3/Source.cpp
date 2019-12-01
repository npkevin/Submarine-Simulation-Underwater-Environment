#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <time.h>

#include <gl/glut.h>
#include <gl/freeglut_ext.h>
#include <gl/freeglut_std.h>
#include <glm.hpp>
#include "PerlinNoise.hpp"

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
void loadAllTextures(void);
unsigned char* readTexel(const char* path);
void pushPremadeBloblist(void);
bool testBlobCollision(void);

void drawLittleBalckSubmarine();
void drawPropellor(int pos);

//Textures
GLuint sandTexture_id;
unsigned char* sand;
GLuint metalTexture_id;
unsigned char* metal;
GLuint glassTexture_id;
unsigned char* glass;


// keyDown flags
bool isDownW = false;
bool isDownS = false;
bool isDownA = false;
bool isDownD = false;
bool isDownSpace = false;
bool isDownC = false;

// Movement stuff

float minAltitude = -5.0;
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
glm::vec3 subPosition = glm::vec3(0.0f, 1.0f, 0.0f);
float zoom = 15.0;

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
const int meshSize = 128; // meshSize x meshSize (quads)
const int meshWidth = 256; 
const int meshLength = 256;

static GLfloat light_position[] = { 100.0F, 100.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// DRONE MATERIALS
static GLfloat drone_mat_ambient[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.01F, 0.01F, 0.01F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.05F, 0.05F, 0.05F, 1.0F };
static GLfloat drone_mat_shininess[] = { 1.0F };
// Blue
GLfloat drone_blade_mat_ambient[] = { 0.2F, 0.4F, 0.6F, 1.0F };
GLfloat drone_blade_mat_specular[] = { 0.02F, 0.4F, 0.6F, 1.0F };
GLfloat drone_blade_mat_diffuse[] = { 0.1F, 0.2F, 0.6F, 1.0F };
GLfloat drone_blade_mat_shininess[] = { 1.0F };

GLfloat noMaterial[] = { 1.0F, 1.0F, 1.0F, 1.0F };

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

	printf("OpenGL Version: %s\n",glGetString(GL_VERSION));
	loadAllTextures();

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

	// Enable Texture-Mapping
	glEnable(GL_TEXTURE_2D);

	// Terrain
	glBindTexture(GL_TEXTURE_2D, sandTexture_id);
	InitMeshQM(&terrain, meshSize, origin, meshWidth, meshLength, dir1v, dir2v);

	// Load Premade blobs
	pushPremadeBloblist();
	
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
	gluLookAt(
		subPosition.x + sin(submarineRotation * DEG2RAD) * zoom, subPosition.y + 8, subPosition.z + cos(submarineRotation * DEG2RAD) * zoom,
		subPosition.x, subPosition.y, subPosition.z,
		0.0, 1.0, 0.0);
}

void displayHandler(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	testBlobCollision();

	glMatrixMode(GL_MODELVIEW);

	// Draw ground mesh
	glBindTexture(GL_TEXTURE_2D, sandTexture_id);
	DrawMeshQM(&terrain, meshSize);
	
	glPushMatrix();
		glTranslatef(subPosition.x, subPosition.y, subPosition.z);
		glRotatef(submarineRotation, 0, 1, 0);
		drawLittleBalckSubmarine();
	glPopMatrix();
	
	glLoadIdentity();
	gluLookAt(
		subPosition.x + sin(submarineRotation * DEG2RAD)*zoom, subPosition.y + 8, subPosition.z + cos(submarineRotation * DEG2RAD)*zoom,
		subPosition.x, subPosition.y, subPosition.z,
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
			subPosition.x -= deltaTime * speed * sinf(submarineRotation * DEG2RAD);
			subPosition.z -= deltaTime * speed * cosf(submarineRotation * DEG2RAD);
		}
		if (isDownS) {
			backPropRotation -= deltaTime * speed * 50;
			leftPropRotation -= deltaTime * speed * 50;
			rightPropRotation -= deltaTime * speed * 50;
			subPosition.x += deltaTime * speed * sinf(submarineRotation * DEG2RAD);
			subPosition.z += deltaTime * speed * cosf(submarineRotation * DEG2RAD);
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
			subPosition.y += deltaTime * speed * fabs(rise_decline_angle / max_rise_angle);
			rise_decline_angle += deltaTime * 0.1;
			if (rise_decline_angle > max_rise_angle) rise_decline_angle = max_rise_angle;
		}
		if (isDownC) {
			subPosition.y -= deltaTime * speed * fabs(rise_decline_angle / min_decline_angle);
			rise_decline_angle -= deltaTime * 0.1;
			if (subPosition.y < minAltitude) subPosition.y = minAltitude;
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
			glutPostRedisplay();

		}
	}

	if (backPropRotation > 360) backPropRotation = 0;
	if (backPropRotation < 0) backPropRotation = 360;
	if (leftPropRotation > 360) leftPropRotation = 0;
	if (leftPropRotation < 0) leftPropRotation = 360;
	if (rightPropRotation > 360) rightPropRotation = 0;
	if (rightPropRotation < 0) rightPropRotation = 360;
	
}

void drawLittleBalckSubmarine() {
	glPushMatrix();
		// Body
		glRotatef(rise_decline_angle, 1, 0, 0);
		glTranslatef(0, 1, 0);
		glScalef(1.0F, 1.0F, 2.0F);
		// Select metal as Texture
		glBindTexture(GL_TEXTURE_2D, metalTexture_id);
		GLUquadricObj* body = gluNewQuadric();
		gluQuadricTexture(body, GL_TRUE);
		gluSphere(body, 1.0F, 16, 16);

		glTranslatef(0, 0, -0.1);
		drawPropellor(0);

		// Window
		glPushMatrix();
			glTranslatef(0, 0.4, -0.1);
			glScalef(1, 1, 0.75);
			GLUquadricObj* window = gluNewQuadric();
			gluSphere(window, 0.8, 16, 16);
		glPopMatrix();

		// Left and right wings/propellors
		glPushMatrix();
			glScalef(0.85, 0.85, 0.85);
			glTranslatef(0, 0, -1);
			// Left propellor
			glPushMatrix();
				glTranslatef(-1.6, 0, 0);
				drawPropellor(1);
			glPopMatrix();
			// Right propellor
			glPushMatrix();
				glTranslatef(1.6, 0, 0);
				drawPropellor(2);
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

void drawPropellor(int pos) {
	// Propellor
	glPushMatrix();
		// Cone thing and back
		glPushMatrix();
			//GLUquadric* propellorShieldBack;
			//propellorShieldBack = gluNewQuadric();
			glTranslatef(0, 0, 1);
			//gluDisk(propellorShieldBack, 0, 0.8, 16, 16);
			glTranslatef(0, 0, 0.001);
			glutSolidCone(0.5, 0.5, 8, 8);
		glPopMatrix();

		glTranslatef(0, 0, 1);

		glBindTexture(GL_TEXTURE_2D, metalTexture_id);
		GLUquadricObj* propellorShield;
		propellorShield = gluNewQuadric();
		gluQuadricTexture(propellorShield, GL_TRUE);
		gluCylinder(propellorShield, 0.8F, 0.8F, 0.5F, 16, 16);

		// Blades
		glPushMatrix();

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
				glRotatef(10, 1, 1, 0);
				gluPartialDisk(blade1, 0, 0.7f, 8, 8, 0, 80);
			glPopMatrix();
			//Blade 2
			glPushMatrix();
				glRotatef(10, 1, -1, 0);
				gluPartialDisk(blade1, 0, 0.7f, 8, 8, 90, 80);
			glPopMatrix();
			//Blade 3
			glPushMatrix();
				glRotatef(10, -1, -1, 0);
				gluPartialDisk(blade1, 0, 0.7f, 8, 8, 180, 80);
			glPopMatrix();
			//Blade 4
			glPushMatrix();
				glRotatef(10, -1, 1, 0);
				gluPartialDisk(blade1, 0, 0.7f, 8, 8, 270, 80);
			glPopMatrix();

			glMaterialfv(GL_FRONT, GL_AMBIENT, noMaterial);
			glMaterialfv(GL_FRONT, GL_SPECULAR, noMaterial);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, noMaterial);
			glMaterialfv(GL_FRONT, GL_SHININESS, noMaterial);

		glPopMatrix();
		// Blades END
	glPopMatrix();
	// Propellor END
}

unsigned char* readTexel(const char * path) {
	unsigned char* texel = (unsigned char*)malloc(2048 * 2048 * 3);;
	// Read image to texel
	FILE* f;
	fopen_s(&f, path, "rb");
	fread(texel, 2048 * 2048 * 3, 1, f);
	return texel;
}

void loadAllTextures(void) {
	
	// Create texture
	glGenTextures(1, &sandTexture_id);
	// Select created texture
	glBindTexture(GL_TEXTURE_2D, sandTexture_id);
	// scan image to memory
	sand = readTexel("./textures/sand.bmp");
	// Texture Parameters (for sandTexture_id)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Prepare texture (for sandTexture_id)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, sand);

	//metal
	glGenTextures(1, &metalTexture_id);
	glBindTexture(GL_TEXTURE_2D, metalTexture_id);
	metal = readTexel("./textures/metal.bmp");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 280, 280, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, metal);

	//window
	glGenTextures(1, &glassTexture_id);
	glBindTexture(GL_TEXTURE_2D, glassTexture_id);
	glass = readTexel("./textures/glass.bmp");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 432, 432, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, glass);
}

void pushPremadeBloblist() {
	// Bloblist premade
	Metaball b0;
	Metaball b1;
	Metaball b2;
	Metaball b3;
	Metaball b4;
	Metaball b5;
	b0.width = 0.02;
	b0.height = 30;
	b0.pos = glm::vec3(128, 0, -128);
	b1.width = 0.02;
	b1.height = 15;
	b1.pos = glm::vec3(20, 0, -154);
	b2.width = 0.02;
	b2.height = 15;
	b2.pos = glm::vec3(190, 0, -111);
	b3.width = 0.02;
	b3.height = 40;
	b3.pos = glm::vec3(200, 0, -26);
	b4.width = 0.02;
	b4.height = -30;
	b4.pos = glm::vec3(55, 0, -200);
	b5.width = 0.02;
	b5.height = 20;
	b5.pos = glm::vec3(14, 0, -53);
	ballList.push_back(b0); ballList.push_back(b1);
	ballList.push_back(b2); ballList.push_back(b3);
	ballList.push_back(b4); ballList.push_back(b5);
	UpdateMesh(&terrain, ballList);
}

bool testBlobCollision(void) {
	float distance;
	float noiseScale = 0.05f;

	for (int i = 0; i < ballList.size(); i++) {
		distance = glm::distance(glm::vec3(subPosition.x, 0, subPosition.z), ballList[i].pos);
		PerlinNoise perl = PerlinNoise(1337);
		float height = 3 * perl.noise(subPosition.x * noiseScale, subPosition.z * noiseScale);
		// Collision detection

		if (subPosition.y < ballList[i].height * exp(-(ballList[i].width * (distance * distance))) + height && ballList[i].height > 0) {
			selfDestruct();
			return true;
		}
	}
	return false;
}

void selfDestruct() {

}