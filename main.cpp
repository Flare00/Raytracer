// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------
#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <algorithm>
#include <chrono>

#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/Scene.h"
#include <GL/glut.h>

#include "src/matrixUtilities.h"

using namespace std;

#include "src/imageLoader.h"
#include "src/Material.h"

#define SAMPLES 5


// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 640;
static unsigned int SCREENHEIGHT = 480;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX = 0, lastY = 0, lastZoom = 0;
static unsigned int FPS = 0;
static bool fullScreen = false;

std::vector<Scene> scenes;
unsigned int selected_scene;

std::vector< std::pair< Vec3, Vec3 > > rays;

void printUsage() {
	cerr << endl
		<< "gMini: a minimal OpenGL/GLUT application" << endl
		<< "for 3D graphics." << endl
		<< "Author : Tamy Boubekeur (http://www.labri.fr/~boubek)" << endl << endl
		<< "Usage : ./gmini [<file.off>]" << endl
		<< "Keyboard commands" << endl
		<< "------------------" << endl
		<< " ?: Print help" << endl
		<< " w: Toggle Wireframe Mode" << endl
		<< " g: Toggle Gouraud Shading Mode" << endl
		<< " f: Toggle full screen mode" << endl
		<< " <drag>+<left button>: rotate model" << endl
		<< " <drag>+<right button>: move model" << endl
		<< " <drag>+<middle button>: zoom" << endl
		<< " q, <esc>: Quit" << endl << endl;
}

void usage() {
	printUsage();
	exit(EXIT_FAILURE);
}




// ------------------------------------
void initLight() {
	GLfloat light_position[4] = { 0.0, 1.5, 0.0, 1.0 };
	GLfloat color[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient[4] = { 1.0, 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT1, GL_SPECULAR, color);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
}

void init() {
	camera.resize(SCREENWIDTH, SCREENHEIGHT);
	initLight();
	//glCullFace (GL_BACK);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
}


// ------------------------------------
// Replace the code of this 
// functions for cleaning memory, 
// closing sockets, etc.
// ------------------------------------

void clear() {

}

// ------------------------------------
// Replace the code of this 
// functions for alternative rendering.
// ------------------------------------


void draw() {
	glEnable(GL_LIGHTING);
	scenes[selected_scene].draw();

	// draw rays : (for debug)
	//  std::cout << rays.size() << std::endl;
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(6);
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	for (unsigned int r = 0; r < rays.size(); ++r) {
		glVertex3f(rays[r].first[0], rays[r].first[1], rays[r].first[2]);
		glVertex3f(rays[r].second[0], rays[r].second[1], rays[r].second[2]);
	}
	glEnd();
}

void display() {
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera.apply();
	draw();
	glFlush();
	glutSwapBuffers();
}

void idle() {
	static float lastTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
	static unsigned int counter = 0;
	counter++;
	float currentTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
	if (currentTime - lastTime >= 1000.0f) {
		FPS = counter;
		counter = 0;
		static char winTitle[64];
		sprintf(winTitle, "Raytracer - FPS: %d", FPS);
		glutSetWindowTitle(winTitle);
		lastTime = currentTime;
	}
	glutPostRedisplay();
}

struct RayTraceImage {
	vector<Vec3> pixels;
	int w, h;
	unsigned int nsamples;
	RayTraceImage(int w, int h, unsigned int nsamples) {
		this->pixels = vector<Vec3>(w * h, Vec3(0, 0, 0));
		this->w = w;
		this->h = h;
		this->nsamples = nsamples;
	}
};


RayTraceImage* image;
Vec3 positionCamera;
Vec3** directionLoader;
int nbLineGroup;
thread * threads;
double * threadProgression;
double totalProgression;

int nbLineThread = 64;


void rayTraceLigne(int id, int minY, int maxY) {
	int counter = 0;
	for(int y = minY; y < maxY; y++){
		for (int x = 0; x < image->w; x++) {
			for (unsigned int s = 0; s < image->nsamples; ++s) {
				Vec3 direction = directionLoader[y][(x * image->nsamples) + s] - positionCamera;
				direction.normalize();
				Vec3 color = scenes[selected_scene].rayTrace(Ray(positionCamera, direction));
				image->pixels[x + y * image->w] += color;
			}
			image->pixels[x + y * image->w] /= image->nsamples;
			counter++;
			threadProgression[id] = ((double)counter / ((maxY-minY)*image->w)) ;
		}
	}
}

void progressionCheck(){
	int lastShow = 0;
	while(totalProgression < 99.9){
		double somme = 0;
		for(int line = 0; line < nbLineGroup; line++){
			somme += threadProgression[line];
		}
		double progression = (somme/nbLineGroup)* 100.0;
		if(totalProgression < progression){
			totalProgression = (progression < 100.0 ? progression : 100.0) ;
			if(lastShow < (int)totalProgression && (int)totalProgression%5 == 0){
				lastShow = (int)totalProgression;	
				cout << lastShow << "%" << endl;
			}
		}
	}
}



void ray_trace_from_camera() {
	int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
	totalProgression = 0;
	std::cout << "Ray tracing a " << w << " x " << h << " image" << std::endl;
	srand(clock());
	camera.apply();
	//unsigned int nsamples = 100;
	image = new RayTraceImage(w, h, SAMPLES);
	time_t temp = time(NULL);
	nbLineGroup = (float)h/nbLineThread + 1;
	threads = new thread[nbLineGroup];
	threadProgression = new double[nbLineGroup];
	positionCamera = cameraSpaceToWorldSpace(Vec3(0, 0, 0));
	directionLoader = new Vec3 * [image->h]{ };
	int avancement = 0;
	int lastAvancement = 0;
	int startY = 0;
	int counterThread = 0;
	for (int y = 0; y < image->h; y++) {
		directionLoader[y] = new Vec3[image->w * image->nsamples];
		for (int x = 0; x < image->w; x++) {
			for (unsigned int s = 0; s < image->nsamples; ++s) {
				float u = ((float)(x)+(float)(rand()) / (float)(RAND_MAX)) / w;
				float v = ((float)(y)+(float)(rand()) / (float)(RAND_MAX)) / h;
				directionLoader[y][(x * image->nsamples) + s] = screen_space_to_worldSpace(u, v);
			}
		}
	}
	for(int line = 0; line < nbLineGroup; line++){
		threadProgression[line] = 0.0;
		threads[line] = thread(rayTraceLigne, line, line*nbLineThread, ((line+1) * nbLineThread < image->h ? (line+1) * nbLineThread : image->h));

	}
	thread t = thread(progressionCheck);
	for (int i = 0; i < nbLineGroup; i++) {
		threads[i].join();
	}
	t.join();

	cout << "Done : " << (time(NULL) - temp) << " s" << endl;

	std::string filename = "./rendu.ppm";
	ofstream f(filename.c_str(), ios::binary);
	if (f.fail()) {
		cout << "Could not open file: " << filename << endl;
		return;
	}
	f << "P3" << std::endl << w << " " << h << std::endl << 255 << std::endl;
	for (int i = 0; i < w * h; i++)
		f << (int)(255.f * std::min<float>(1.f, image->pixels[i][0])) << " " << (int)(255.f * std::min<float>(1.f, image->pixels[i][1])) << " " << (int)(255.f * std::min<float>(1.f, image->pixels[i][2])) << " ";
	f << std::endl;
	f.close();

}


void key(unsigned char keyPressed, int x, int y) {
	Vec3 pos, dir;
	thread t;
	switch (keyPressed) {
	case 'f':
		if (fullScreen == true) {
			glutReshapeWindow(SCREENWIDTH, SCREENHEIGHT);
			fullScreen = false;
		}
		else {
			glutFullScreen();
			fullScreen = true;
		}
		break;
	case 'q':
	case 27:
		clear();
		exit(0);
		break;
	case 'w':
		GLint polygonMode[2];
		glGetIntegerv(GL_POLYGON_MODE, polygonMode);
		if (polygonMode[0] != GL_FILL)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;

	case 'r':
		camera.apply();
		rays.clear();
		ray_trace_from_camera();

		break;
	case '+':
		selected_scene++;
		if (selected_scene >= scenes.size()) selected_scene = 0;
		break;
	default:
		printUsage();
		break;
	}
	idle();
}

void mouse(int button, int state, int x, int y) {
	if (state == GLUT_UP) {
		mouseMovePressed = false;
		mouseRotatePressed = false;
		mouseZoomPressed = false;
	}
	else {
		if (button == GLUT_LEFT_BUTTON) {
			camera.beginRotate(x, y);
			mouseMovePressed = false;
			mouseRotatePressed = true;
			mouseZoomPressed = false;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			lastX = x;
			lastY = y;
			mouseMovePressed = true;
			mouseRotatePressed = false;
			mouseZoomPressed = false;
		}
		else if (button == GLUT_MIDDLE_BUTTON) {
			if (mouseZoomPressed == false) {
				lastZoom = y;
				mouseMovePressed = false;
				mouseRotatePressed = false;
				mouseZoomPressed = true;
			}
		}
	}
	idle();
}

void motion(int x, int y) {
	if (mouseRotatePressed == true) {
		camera.rotate(x, y);
	}
	else if (mouseMovePressed == true) {
		camera.move((x - lastX) / static_cast<float>(SCREENWIDTH), (lastY - y) / static_cast<float>(SCREENHEIGHT), 0.0);
		lastX = x;
		lastY = y;
	}
	else if (mouseZoomPressed == true) {
		camera.zoom(float(y - lastZoom) / SCREENHEIGHT);
		lastZoom = y;
	}
}


void reshape(int w, int h) {
	camera.resize(w, h);
}





int main(int argc, char** argv) {
	if (argc > 2) {
		printUsage();
		exit(EXIT_FAILURE);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(SCREENWIDTH, SCREENHEIGHT);
	window = glutCreateWindow("gMini");

	init();
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	key('?', 0, 0);

	selected_scene = 3;
	scenes.resize(5);
	scenes[0].setup_single_sphere();
	scenes[1].setup_single_square();
	scenes[2].setup_double_sphere();
	scenes[3].setup_cornellBox();

	scenes[4].setup_cornellBoxMesh();
	glutMainLoop();
	return EXIT_SUCCESS;
}
