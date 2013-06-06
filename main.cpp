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
static bool readyToStart(false);
static float win_width, win_height;
static string currPlayer, ipAddress;

void GLFWCALL KeyCallback(int key, int action) {
	if (key == GLFW_KEY_ESC) {
		glfwCloseWindow();
		return;
	}

	if (scene && currPlayer[0] == '1') {
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
	}
	if (scene) Networking::KeyAction(key, action, scene->GetShipOffset());
	TwEventKeyGLFW(key, action);
}

void GLFWCALL MouseCallback(int x, int y) {
	// This gets called once before the window has been
	// initialized; the if block makes sure we don't
	// preset theta and phi to junk when that happens
	if (win_width > 0 && win_height > 0 && currPlayer[0] == '2' && scene)
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
	if (currPlayer[0] == '2' && scene) {
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
		scene->UpdateFBO(w, h);
	}

	// Update global
	win_width = w;
	win_height = h;
	TwWindowSize(w, h);
}

// Function called to copy the content of a std::string (souceString) handled 
// by the AntTweakBar library to destinationClientString handled by our application
void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString) {
	destinationClientString = sourceLibraryString;
}

void TW_CALL StartGameCB(void *_gameInfo) {
	pair<string, string> gameInfo = *(pair<string, string> *)_gameInfo;
	cout << "button clicked " << gameInfo.first << gameInfo.second << endl;
	ipAddress = gameInfo.first;
	currPlayer = gameInfo.second;
	readyToStart = true;
}

static void loadScene() {
	TwDeleteAllBars();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Level *level = new Level();

	// Choose player
	Player p;
	//currPlayer = argv[2][0];
	switch (currPlayer[0]) {
		//switch('1') { // TODO: change for final
	case '1':
		p = PLAYER1;
		break;
	case '2':
		p = PLAYER2;
		break;
	}
	scene = new Scene(p);
	Networking::Init(scene, level, ipAddress, currPlayer.c_str());
	scene->LoadLevel(level);
	WindowResizeCallback(win_width, win_height);
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
int main(int argc, char *argv[])
{
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

	float bgColor[] = { 0.1f, 0.2f, 0.4f };         // Background color 

	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);

	// Create a tweak bar
	TwBar *bar = TwNewBar("TweakBar");
	//TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.
	TwDefine(" TweakBar movable=false resizable=false iconifiable=false fontresizable=false ");

	// Define the required callback function to copy a std::string (see TwCopyStdStringToClientFunc documentation)
	TwCopyStdStringToClientFunc(CopyStdStringToClient);
	pair<string, string> gameInfo("localhost", "1s");
	if (argc > 1) gameInfo.first = argv[1];
	if (argc > 2) gameInfo.second = argv[2];
	TwAddVarRW(bar, "IP", TW_TYPE_STDSTRING, &gameInfo.first, " label='Server IP Address' ");
	TwAddVarRW(bar, "Player", TW_TYPE_STDSTRING, &gameInfo.second, " label='Player Number' ");

	TwAddButton(bar, "Start", StartGameCB, &gameInfo, " label='Start Game' ");

	// - Directly redirect GLFW char events to AntTweakBar
	glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);

	glfwSetKeyCallback(KeyCallback);
	glfwSetMousePosCallback(MouseCallback);
	glfwSetMouseButtonCallback(MouseButtonCallback);

	glfwSetWindowTitle("CS248 Project");
	glfwSetWindowSizeCallback(WindowResizeCallback);

	glEnable(GL_DEPTH_TEST);    // Depth testing
	glEnable(GL_LINE_SMOOTH);   // Smooth lines
	glEnable(GL_MULTISAMPLE);   // Multisampling
	glEnable(GL_TEXTURE_2D);    // Texturing
	glLineWidth(2.0f);          // Set line width

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
		if (scene) scene->Render();
		else if (readyToStart) loadScene();
		TwDraw();
		glfwSwapBuffers();
	}
	TwTerminate();
	glfwTerminate();
	return 0;
}