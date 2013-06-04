#include "gl.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif
#include <glm/glm.hpp>
#include <time.h>
#include <AntTweakBar.h>

#include "Scene.h"
#include "Level.h"
#include "Networking.h"

using namespace std;
using namespace glm;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

static Scene *scene;
static float win_width, win_height;
static string currPlayer;

void GLFWCALL KeyCallback(int key, int action) {
	switch(key) {
	case GLFW_KEY_ESC:
		glfwCloseWindow();
		break;
	case GLFW_KEY_LEFT:
		scene->keyLeft = (action == GLFW_PRESS);
		break;
	case GLFW_KEY_RIGHT:
		scene->keyRight = (action == GLFW_PRESS);
		break;
	case GLFW_KEY_UP:
		scene->keyUp = (action == GLFW_PRESS);
		break;
	case GLFW_KEY_DOWN:
		scene->keyDown = (action == GLFW_PRESS);
		break;
	}
	Networking::KeyAction(key, action, scene->GetShipOffset());
	TwEventKeyGLFW(key, action);
}

void GLFWCALL MouseCallback(int x, int y) {
	// This gets called once before the window has been
	// initialized; the if block makes sure we don't
	// preset theta and phi to junk when that happens
	if (win_width > 0 && win_height > 0 && currPlayer[0] == '2')
	{
		glfwSetMousePos(win_width / 2, win_height / 2);

		// Compute new orientation
		scene->theta += 0.005 * (win_width / 2 - x);
		scene->phi   += 0.005 * (win_height / 2 - y);

		// Clamp up-down
		if (scene->phi > M_PI / 2)
			scene->phi = M_PI / 2;
		else if (scene->phi < -M_PI / 2)
			scene->phi = -M_PI / 2;
	}
	TwEventMousePosGLFW(x, y);
}

void GLFWCALL MouseButtonCallback(int button, int action)
{
	if (currPlayer[0] == '2') {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS)
				scene->mouseLeft = true;
			else if (action == GLFW_RELEASE)
				scene->mouseLeft = false;
			else
				cerr << "Unsupported mouse action " << endl;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)
				scene->mouseRight = true;
			else if (action == GLFW_RELEASE)
				scene->mouseRight = false;
			else
				cerr << "Unsupported mouse action " << endl;
		}
		else {
			cerr << "Unsupported mouse button " << endl;
		}
	}
	TwEventMouseButtonGLFW(button, action);
}

void GLFWCALL WindowResizeCallback(int w, int h)
{
	// Update viewable area
	glViewport(0, 0, w, h);

	// Update projection matrix
	float ratio = (float)w / h;
	if (scene) {
		scene->SetProjection(glm::perspective(75.0f,        // Field of view
			ratio,        // Aspect ratio
			0.1f,         // Near clipping plane
			50.0f));     // Far clipping plane
		scene->SetFrustum(75.0f, ratio, 0.1f, 50.0f);
	}

	// Update global
	win_width = w;
	win_height = h;
	glfwSetMousePos(win_width / 2, win_height / 2);
	TwWindowSize(w, h);
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
int main(int argc, char *argv[])
{
	if (argc < 3) {
		cout << "Usage: EndersGame [server ip address] [player number]" << endl;
		exit(-1);
	}

	if (!glfwInit()) {
		cerr << "Failed to initialize glfw" << endl;
		exit(-1);
	}

	// using opengl version 2.1
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

#ifdef __APPLE__
	CGSetLocalEventsSuppressionInterval(0.0);
#endif

	if (!glfwOpenWindow(1024, 768, 5, 6, 5, 0, 24, 0, GLFW_WINDOW)) {
		cerr << "Failed to initialize glfw window" << endl;
		glfwTerminate();
		exit(-1);
	}
	glfwEnable(GLFW_MOUSE_CURSOR);
	glfwEnable(GLFW_KEY_REPEAT);

#ifndef __APPLE__
	glewInit();
#endif

	double currTime = 0, dt;// Current time and enlapsed time
	double turn = 0;    // Model turn counter
	double speed = 0.3; // Model rotation speed
	int wire = 0;       // Draw model in wireframe?
	float bgColor[] = { 0.1f, 0.2f, 0.4f };         // Background color 
	unsigned char cubeColor[] = { 255, 0, 0, 128 }; // Model color (32bits RGBA)

	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);

	// Create a tweak bar
	TwBar *bar = TwNewBar("TweakBar");
	TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.
	TwDefine(" TweakBar movable=false resizable=false iconifiable=false fontresizable=false ");

	// Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
	TwAddVarRW(bar, "speed", TW_TYPE_DOUBLE, &speed, 
		" label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed (turns/second)' ");

	// Add 'wire' to 'bar': it is a modifable variable of type TW_TYPE_BOOL32 (32 bits boolean). Its key shortcut is [w].
	TwAddVarRW(bar, "wire", TW_TYPE_BOOL32, &wire, 
		" label='Wireframe mode' key=w help='Toggle wireframe display mode.' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarRO(bar, "time", TW_TYPE_DOUBLE, &currTime, " label='Time' precision=1 help='Time (in seconds).' ");         

	// Add 'bgColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
	TwAddVarRW(bar, "bgColor", TW_TYPE_COLOR3F, &bgColor, " label='Background color' ");

	// Add 'cubeColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR32 (32 bits color) with alpha
	TwAddVarRW(bar, "cubeColor", TW_TYPE_COLOR32, &cubeColor, 
		" label='Cube color' alpha help='Color and transparency of the cube.' ");

	// - Directly redirect GLFW mouse button events to AntTweakBar
	//glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW);
	// - Directly redirect GLFW mouse position events to AntTweakBar
	//glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
	// - Directly redirect GLFW mouse wheel events to AntTweakBar
	//glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
	// - Directly redirect GLFW key events to AntTweakBar
	//glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
	// - Directly redirect GLFW char events to AntTweakBar
	glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);

	Level *level = new Level();

	// Choose player
	Player p;
	currPlayer = argv[2][0];
	switch (argv[2][0]) {
		//switch('1') { // TODO: change for final
	case '1':
		p = PLAYER1;
		glfwSetKeyCallback(KeyCallback);
		glfwSetMousePosCallback(MouseCallback);
		glfwSetMouseButtonCallback(MouseButtonCallback);
		break;
	case '2':
		p = PLAYER2;
		glfwSetMousePosCallback(MouseCallback);
		glfwSetMouseButtonCallback(MouseButtonCallback);
		break;
	}
	scene = new Scene(p);
	Networking::Init(scene, level, argv[1], argv[2]);
	scene->LoadLevel(level);

	glfwSetWindowTitle("CS248 Project");
	glfwSetWindowSizeCallback(WindowResizeCallback);

	glEnable(GL_DEPTH_TEST);    // Depth testing
	glEnable(GL_LINE_SMOOTH);   // Smooth lines
	glEnable(GL_MULTISAMPLE);   // Multisampling
	glEnable(GL_TEXTURE_2D);    // Texturing

	// Seed random
	srand(time(NULL));

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// Main render loop
	while(glfwGetWindowParam(GLFW_OPENED)) {
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){
			cout << nbFrames << " fps" << endl;
			nbFrames = 0;
			lastTime += 1.0;
		}
		scene->Render();
		TwDraw();
		glfwSwapBuffers();
	}
	TwTerminate();


	glfwTerminate();
	return 0;
}