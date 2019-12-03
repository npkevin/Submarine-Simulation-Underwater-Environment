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
#define RAD2DEG 180.0f/3.14159f

typedef struct Player
{
	glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
	float backPropRotation = 0.0f;
	float leftPropRotation = 0.0f;
	float rightPropRotation = 0.0f;
	float submarineRotation = (((double)rand() / (RAND_MAX)) + 1) * 360;
	float rise_decline_angle = 0.0f;
	float speed = 0.01f;
	float breakApart = 0.0f;
	bool isDead = false;
	bool isEnemy = true;
	float sight = 50.0f;
	int fireCooldown = 0;
	 
	glm::vec3 getForward() {
		glm::vec3 forward = glm::vec3(glm::vec3(sinf(submarineRotation * DEG2RAD), 0, cosf(submarineRotation * DEG2RAD)));
		return glm::normalize(-forward);
	}

} Player;

typedef struct Torpedo
{
	glm::vec3 position;
	glm::vec3 forward;
	float angle;
} Torpedo;

void initOpenGL(int, int);
void displayHandler(void);
void reshapeHandler(int, int);
void keyboardInputHandler(unsigned char, int, int);
void mouseButtonHandler(int, int, int, int);
void mouseMotionHandler(int, int);
void functionKeys(int key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void idle();
void loadAllTextures(void);
unsigned char* readTexel(const char* path);
void pushPremadeBloblist(void);
bool testBlobCollision(void);
void selfDestruct(Player *p);
void newTorpedo(Player p);
void drawTorpedo(Torpedo t);
bool torpedoCollision(void);
bool submarineCollision(void);

void drawSub(Player p);
void drawPropellor(int pos, Player p);

//Textures
GLuint sandTexture_id;
unsigned char* sand;
GLuint metalTexture_id;
unsigned char* metal;
GLuint glassTexture_id;
unsigned char* glass;
GLuint redmetalTexture_id;
unsigned char* rmetal;


// keyDown flags
bool isDownW = false;
bool isDownS = false;
bool isDownA = false;
bool isDownD = false;
bool isDownSpace = false;
bool isDownC = false;


// Player
Player player;
std::vector<Player> enemies;
std::vector<Torpedo> torpedos;


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
float minAltitude = -5.0;
float max_rise_angle = 20.0f;
float min_decline_angle = -20.0f;
float zoom = 20.0;
float minSpeed = 0.002;
float maxSpeed = 0.05;
int npcAiUpdateTime = 0;
int timer = CLOCKS_PER_SEC * 5;
int fireSpeed = CLOCKS_PER_SEC * 0.6;

// Terrain
static QuadMesh terrain;
const int meshSize = 128; // meshSize x meshSize (quads)
const int meshWidth = 256; 
const int meshLength = 256;

static GLfloat light_position[] = { 100.0F, 100.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };



static GLfloat explosion_mat_ambient[] = {1.0f, 0.0, 0.0, 1.0f};
static GLfloat explosion_mat_specular[] = { 0.01F, 0.01F, 0.01F, 1.0F };
static GLfloat explosion_mat_diffuse[] = { 0.05F, 0.05F, 0.05F, 1.0F };
static GLfloat explosion_mat_shininess[] = { 1.0F };


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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
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
	glutSpecialFunc(functionKeys);

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	// Player settings
	player.position = glm::vec3(128, 2, -50);
	player.isEnemy = false;
	player.speed = 0.02;

	// Premade enemy list
	Player npc1, npc2, npc3, npc4, npc5, npc6;
	npc1.position = glm::vec3(120, 3, -120);
	npc2.position = glm::vec3(125, 3, -105);
	npc3.position = glm::vec3(130, 3, -120);
	npc4.position = glm::vec3(135, 3, -125);
	npc5.position = glm::vec3(120, 2, -130);
	npc6.position = glm::vec3(125, 3, -155);
	enemies.push_back(npc1); 
	enemies.push_back(npc2); enemies.push_back(npc3); enemies.push_back(npc4); enemies.push_back(npc5); enemies.push_back(npc6);

	
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
		player.position.x + sin(player.submarineRotation * DEG2RAD) * zoom, player.position.y + 8, player.position.z + cos(player.submarineRotation * DEG2RAD) * zoom,
		player.position.x, player.position.y, player.position.z,
		0.0, 1.0, 0.0);
}

void displayHandler(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	testBlobCollision();
	torpedoCollision();
	submarineCollision();


	glMatrixMode(GL_MODELVIEW);

	// Draw ground mesh
	glBindTexture(GL_TEXTURE_2D, sandTexture_id);
	DrawMeshQM(&terrain, meshSize);


	glPushMatrix();
		glTranslatef(player.position.x, player.position.y, player.position.z);
		glRotatef(player.submarineRotation, 0, 1, 0);
		drawSub(player);
	glPopMatrix();

	// Draw enemies
	for (int i = 0; i < enemies.size() ; i++) {
		glPushMatrix();
			glTranslatef(enemies[i].position.x, enemies[i].position.y, enemies[i].position.z);
			glRotatef(enemies[i].submarineRotation, 0, 1, 0);
			drawSub(enemies[i]);
		glPopMatrix();
	}

	// Draw torpedos
	for (int i = 0; i < torpedos.size(); i++) {
		drawTorpedo(torpedos[i]);
	}


	glLoadIdentity();
	gluLookAt(
		player.position.x + sin(player.submarineRotation * DEG2RAD) * zoom, player.position.y + 9, player.position.z + cos(player.submarineRotation * DEG2RAD) * zoom,
		player.position.x, player.position.y, player.position.z,
		0.0, 1.0, 0.0);
	glutSwapBuffers();
}
// state:0 == keyDown
void mouseButtonHandler(int button, int state, int x, int y) {

	
}

void mouseMotionHandler(int x, int y) {

	
}


void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{
		printf("A/D Key: Rotate\n");
		printf("W/S Key: Move Forward/Backwards\n");
		printf("Enter Key: Shoot Torpedo\n");
		printf("=/- Key: Change Speed\n");
		printf("C Key: Lower the submarine\n");
		printf("Spacebar: Raise the submarine\n");

	}
}

void keyboardInputHandler(unsigned char key, int x, int y) {
	if (!player.isDead) {
		switch (key) {
			//esc
		case 27:
			glutDestroyWindow(mainWindowID);
			break;
		case 13:
			newTorpedo(player);
			break;
		case '-':
			player.speed -= 0.002;
			if (player.speed < minSpeed) player.speed = minSpeed;
			printf("speed: %f\n", player.speed);
			break;
		case '=':
			player.speed += 0.002;
			if (player.speed > maxSpeed) player.speed = maxSpeed;
			printf("speed: %f\n", player.speed);
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
	switch (key) {
	case 'r':
		player.isDead = false;
		player.position = glm::vec3(0.0f, 2.0f, 0.0f);
		player.breakApart = 0;
		glutPostRedisplay();
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

	if (isDownW || isDownS && !player.isDead) {
		if (isDownW && !player.isDead) {
			player.backPropRotation += deltaTime * player.speed * 50;
			player.leftPropRotation += deltaTime * player.speed * 50;
			player.rightPropRotation += deltaTime * player.speed * 50;
			player.position += (float)(deltaTime * player.speed) * player.getForward();
		}
		if (isDownS && !player.isDead) {
			player.backPropRotation -= deltaTime * player.speed * 50;
			player.leftPropRotation -= deltaTime * player.speed * 50;
			player.rightPropRotation -= deltaTime * player.speed * 50;
			player.position -= (float)(deltaTime * player.speed) * player.getForward();
		}
		glutPostRedisplay();
	}
	if (isDownA || isDownD && !player.isDead) {
		if (isDownA && !player.isDead) {
			if (player.submarineRotation > 360) player.submarineRotation = 0;
			player.submarineRotation += deltaTime * player.speed * 10;
			player.leftPropRotation -= deltaTime * player.speed * 50;
			player.rightPropRotation += deltaTime * player.speed * 50;
		}
		if (isDownD && !player.isDead) {
			if (player.submarineRotation < 0) player.submarineRotation = 360;
			player.submarineRotation -= deltaTime * player.speed * 10;
			player.leftPropRotation += deltaTime * player.speed * 50;
			player.rightPropRotation -= deltaTime * player.speed * 50;
		}
		glutPostRedisplay();
	}
	if (isDownSpace || isDownC && !player.isDead) {
		if (isDownSpace) {
			player.position.y += deltaTime * player.speed * fabs(player.rise_decline_angle / max_rise_angle);
			player.rise_decline_angle += deltaTime * 0.1;
			if (player.rise_decline_angle > max_rise_angle) player.rise_decline_angle = max_rise_angle;
		}
		if (isDownC && !player.isDead) {
			player.position.y -= deltaTime * player.speed * fabs(player.rise_decline_angle / min_decline_angle);
			player.rise_decline_angle -= deltaTime * 0.1;
			if (player.position.y < minAltitude) player.position.y = minAltitude;
			if (player.rise_decline_angle < min_decline_angle) player.rise_decline_angle = min_decline_angle;
		}
		player.backPropRotation += deltaTime * 0.2;
		player.leftPropRotation += deltaTime * 0.2;
		player.rightPropRotation += deltaTime * 0.4;
		glutPostRedisplay();
	}
	else {
		if (player.rise_decline_angle != 0) {
			if (player.rise_decline_angle > 0) {
				player.rise_decline_angle -= deltaTime * 0.1;
				if (player.rise_decline_angle < 0) player.rise_decline_angle = 0;
			}
			if (player.rise_decline_angle < 0) {
				player.rise_decline_angle += deltaTime * 0.1;
				if (player.rise_decline_angle > 0) player.rise_decline_angle = 0;
			}
			glutPostRedisplay();

		}
	}

	// Limits
	if (player.backPropRotation > 360) player.backPropRotation = 0;
	if (player.backPropRotation < 0) player.backPropRotation = 360;
	if (player.leftPropRotation > 360) player.leftPropRotation = 0;
	if (player.leftPropRotation < 0) player.leftPropRotation = 360;
	if (player.rightPropRotation > 360) player.rightPropRotation = 0;
	if (player.rightPropRotation < 0) player.rightPropRotation = 360;

	// enemySubmarine AI (very dumb), every 5 seconds change direction
	if ((float)now - npcAiUpdateTime > timer) {
		npcAiUpdateTime = now;
		for (int i = 0; i < enemies.size(); i++) {
			if (!enemies[i].isDead) enemies[i].submarineRotation = (((double)rand() / (RAND_MAX)) + 1) * 360;
		}
	}
	else {
		for (int i = 0; i < enemies.size(); i++) {

			enemies[i].backPropRotation += deltaTime * enemies[i].speed * 50;
			enemies[i].leftPropRotation += deltaTime * enemies[i].speed * 50;
			enemies[i].rightPropRotation += deltaTime * enemies[i].speed * 50;

			// Player in range of this sub
			if (glm::distance(player.position, enemies[i].position) < enemies[i].sight && !enemies[i].isDead) {
				glm::vec3 towardsPlayer = glm::vec3(player.position.x - enemies[i].position.x, player.position.y - enemies[i].position.y, player.position.z - enemies[i].position.z);
				towardsPlayer = glm::normalize(towardsPlayer);

				// Face player (visual)
				float towardsPlayerDegree = atan2((float)towardsPlayer.z, -(float)towardsPlayer.x) * RAD2DEG + 180 - 90;
				enemies[i].submarineRotation = towardsPlayerDegree;
				// Move towards player
				enemies[i].position += (float)(deltaTime * enemies[i].speed) * towardsPlayer;

				// Firing range
				if (glm::distance(player.position, enemies[i].position) < enemies[i].sight/2 && !enemies[i].isDead) {
					if (now - enemies[i].fireCooldown > fireSpeed) {
						enemies[i].fireCooldown = now;
						newTorpedo(enemies[i]);
					}
				}
			}
			else {
				if (!enemies[i].isDead) enemies[i].position += (float)(deltaTime * enemies[i].speed) * enemies[i].getForward();
			}
			
			glutPostRedisplay();
		}
	}


	// Check if player is dead
	if (player.isDead) {
		selfDestruct(&player);
	}
	// Check if other subs are dead
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i].isDead) {
			selfDestruct(&enemies[i]);
		}
		if (enemies[i].breakApart > 10) {
			enemies.erase(enemies.begin() + i);
			float randomXPos = (rand() % (156 - 100 + 1)) + 100;
			float randomZPos = (rand() % (156 - 100 + 1)) + 100;
			float randomYPos = (rand() % (5 - 3 + 1)) + 5;
			Player npc;
			printf("%f %f %f \n", randomXPos, randomYPos, randomZPos);
			npc.position = glm::vec3(randomXPos, randomYPos, -randomZPos);
			enemies.push_back(npc);
		}
	}
	
	// Updates torpedo positions
	for (int i = 0; i < torpedos.size(); i++) {
		//torpedos[i].position.x -= deltaTime * sinf(torpedos[i].angle * DEG2RAD) * 0.1;
		//torpedos[i].position.z -= deltaTime * cosf(torpedos[i].angle * DEG2RAD) * 0.1;
		torpedos[i].position += torpedos[i].forward * (float)deltaTime * 0.1f;
		glutPostRedisplay();
	}
	
}

void drawSub(Player p) {
	glPushMatrix();
	GLUquadricObj* body = gluNewQuadric();
	if (p.isDead) {
		float explosion = 0;
		if (p.breakApart <= 5.0) {
			explosion = p.breakApart * 0.5 + 1.0;
		}
		else  {
			explosion = p.breakApart * 0.1 + 3.0;
		}
		
		glMaterialfv(GL_FRONT, GL_AMBIENT, explosion_mat_ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, explosion_mat_specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, explosion_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SHININESS, explosion_mat_shininess);
		gluSphere(body, explosion, 16, 16);
		
	}
		glPushMatrix();
			// Body
			glRotatef(p.rise_decline_angle, 1, 0, 0);
			glTranslatef(0, 1 - p.breakApart, 0);
			glScalef(1.0F, 1.0F, 2.0F);
			// Select metal as Texture
			if (p.isEnemy) {
				glBindTexture(GL_TEXTURE_2D, redmetalTexture_id);
			}
			else {
				glBindTexture(GL_TEXTURE_2D, metalTexture_id);
			}
			//GLUquadricObj* body = gluNewQuadric();
			gluQuadricTexture(body, GL_TRUE);
			gluSphere(body, 1.0F, 16, 16);

			glTranslatef(0, 0, -0.1);
			drawPropellor(0, p);

			// Window
			glPushMatrix();
				glTranslatef(0, 0.4, -0.1);
				glScalef(1, 1, 0.75);
				glBindTexture(GL_TEXTURE_2D, glassTexture_id);
				GLUquadricObj* window = gluNewQuadric();
				gluQuadricTexture(window, GL_TRUE);
				gluSphere(window, 0.8, 8, 8);
			glPopMatrix();
		glPopMatrix();

		// Left and right wings/propellors
		glScalef(1.0F, 1.0F, 2.0F);
		glTranslatef(0, 1, -0.1);
		glPushMatrix();
			glScalef(0.85, 0.85, 0.85);
			glTranslatef(0, 0, -1);
			// Left propellor
			glPushMatrix();
				// Fall diagonal left
				glTranslatef(-1.6 - p.breakApart, -p.breakApart, 0);
				drawPropellor(1, p);
			glPopMatrix();
			// Right propellor
			glPushMatrix();
				// Fall diagonal right
				glTranslatef(1.6 + p.breakApart, -p.breakApart, 0);
				drawPropellor(2, p);
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

void drawPropellor(int pos, Player p) {
	if (p.isEnemy) {
		glBindTexture(GL_TEXTURE_2D, redmetalTexture_id);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, metalTexture_id);
	}
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

		GLUquadricObj* propellorShield;
		propellorShield = gluNewQuadric();
		gluQuadricTexture(propellorShield, GL_TRUE);
		gluCylinder(propellorShield, 0.8F, 0.8F, 0.5F, 16, 16);

		// Blades
		glPushMatrix();

			glTranslatef(0, 0, 0.3);
			float propellorRotation = 0.0;

			if (pos == 0) {
				propellorRotation = p.backPropRotation;
			}
			else if (pos == 1) {
				propellorRotation = p.leftPropRotation;
			}
			else if (pos == 2) {
				propellorRotation = p.rightPropRotation;
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

void drawTorpedo(Torpedo t) {
	glPushMatrix();
		glTranslatef(t.position.x, t.position.y, t.position.z);
		glRotatef(t.angle, 0, 1, 0);
		glBindTexture(GL_TEXTURE_2D, redmetalTexture_id);
		GLUquadricObj* body = gluNewQuadric();
		gluQuadricTexture(body, GL_TRUE);
		gluCylinder(body, 0.2, 0.2, 2, 4, 1);
	glPopMatrix();
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2048, 2048, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, sand);

	//metal
	glGenTextures(1, &metalTexture_id);
	glBindTexture(GL_TEXTURE_2D, metalTexture_id);
	metal = readTexel("./textures/metal.bmp");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 280, 280, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, metal);

	//window
	glGenTextures(1, &glassTexture_id);
	glBindTexture(GL_TEXTURE_2D, glassTexture_id);
	glass = readTexel("./textures/glass.bmp");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 432, 432, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, glass);

	//window
	glGenTextures(1, &redmetalTexture_id);
	glBindTexture(GL_TEXTURE_2D, redmetalTexture_id);
	rmetal = readTexel("./textures/redmetal.bmp");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 300, 300, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, rmetal);
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
		distance = glm::distance(glm::vec3(player.position.x, 0, player.position.z), ballList[i].pos);
		PerlinNoise perl = PerlinNoise(1337);
		float height = 3 * perl.noise(player.position.x * noiseScale, player.position.z * noiseScale);
		// Collision detection

		if (player.position.y < ballList[i].height * exp(-(ballList[i].width * (distance * distance))) + height && ballList[i].height > 0) {
			player.isDead = true;
			return true;
		}
	}
	return false;
}

bool torpedoCollision(void) {
	//Player Collision
	for (int i = 0; i < torpedos.size(); i++) {
		if (player.position.y < torpedos[i].position.y + 1.5 && player.position.y > torpedos[i].position.y - 1.5 && 
			player.position.x < torpedos[i].position.x + 1 && player.position.x > torpedos[i].position.x - 1 &&
			player.position.z < torpedos[i].position.z + 1 && player.position.z > torpedos[i].position.z - 1) {
			player.isDead = true;
			return true;
		}
	}
	//Enemy Collision
	for (int i = 0; i < torpedos.size(); i++) {
		for (int j = 0; j < enemies.size(); j++) {
			if (enemies[j].position.y < torpedos[i].position.y + 1.5 && enemies[j].position.y > torpedos[i].position.y - 1.5 &&
				enemies[j].position.x < torpedos[i].position.x + 1 && enemies[j].position.x > torpedos[i].position.x - 1 &&
				enemies[j].position.z < torpedos[i].position.z + 1 && enemies[j].position.z > torpedos[i].position.z - 1 && !enemies[j].isDead) {
				enemies[j].isDead = true;
				torpedos.erase(torpedos.begin() + i);
				return true;
			}
		}
	}
	return false;
}

bool submarineCollision(void) {
	for (int j = 0; j < enemies.size(); j++) {
		if (enemies[j].position.y < player.position.y + 2 && enemies[j].position.y > player.position.y - 2 &&
			enemies[j].position.x < player.position.x + 2 && enemies[j].position.x > player.position.x - 2 &&
			enemies[j].position.z < player.position.z + 4 && enemies[j].position.z > player.position.z - 4 && !enemies[j].isDead) {
			enemies[j].isDead = true;
			player.isDead = true;
			return true;
		}
	}
}

void selfDestruct(Player *p) {
	p->breakApart += deltaTime * 0.005;
	glutPostRedisplay();
}

void newTorpedo(Player p) {
	Torpedo newTorpedo;
	newTorpedo.forward = p.getForward();
	// Move torpedo spawn forward
	newTorpedo.position = p.position + newTorpedo.forward * 3.0f;
	newTorpedo.angle = p.submarineRotation;
	torpedos.push_back(newTorpedo);
}
