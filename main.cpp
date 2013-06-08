#include "gl.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <glm/glm.hpp>
#include <time.h>
#include <fmod.hpp>

#include "Scene.h"
#include "HUD.h"
#include "Menu.h"
#include "Level.h"
#include "Networking.h"
#include "Sound.h"

// memory leak detection
//#include <vld.h>

using namespace std;
using namespace glm;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

static Menu *menu, *start, *credits, *hscores;
static Scene *scene;
static HUD *hud;
static Player p;

TextField *ipField;
TextField *playerField;

bool netInit;
bool finished;
bool gaming;

static float win_width, win_height;

void GLFWCALL KeyCallback(int key, int action) {
	if (scene && gaming && p == PLAYER1) {
		switch(key) {
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
	}
    if (menu) menu->HandleKey(key, action);
}

void GLFWCALL CharCallback(int character, int action)
{
    if (menu) menu->HandleChar(character, action);
}

void GLFWCALL MouseCallback(int x, int y) {
    if (p == PLAYER1)
        return;
    
	// This gets called once before the window has been
	// initialized; the if block makes sure we don't
	// preset theta and phi to junk when that happens
	if (win_width > 0 && win_height > 0 && scene)
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
}

void GLFWCALL MouseButtonCallback(int button, int action)
{
    if (p == PLAYER1)
        return;
    
	if (scene) {
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
                                              50.0f));      // Far clipping plane
        scene->SetMinimapProjection(glm::ortho(10.0f, -10.0f,   // Left, right
                                               -10.0f, 10.0f,   // Bottom, top
                                               -50.0f, 50.0f)); // Near, far
		scene->SetFrustum(75.0f, ratio, 0.1f, 50.0f);
		scene->UpdateFBO(w, h);
	}
    if (hud) {
        hud->SetWidth(w);
        hud->SetHeight(h);
    }
    if (menu) {
        menu->SetWidth(w);
        menu->SetHeight(h);
    }
    if (start) {
        start->SetWidth(w);
        start->SetHeight(h);
    }
    if (credits) {
        credits->SetWidth(w);
        credits->SetHeight(h);
    }
    if (hscores) {
        hscores->SetWidth(w);
        hscores->SetHeight(h);
    }

	// Update global
	win_width = w;
	win_height = h;
}

void StartGame(void *data)
{
    cout << "Start game! " << endl;
    gaming = true;
    
    Level *level = new Level();
    
    // Choose player
	switch (playerField->GetCurrentText()[0]) {
        case '1':
            p = PLAYER1;
            break;
        case '2':
            p = PLAYER2;
            break;
	}
    
    if (!scene)
        scene = new Scene();
    
    if (!hud)
        hud = new HUD();
    
    Networking::Init(scene, level, ipField->GetCurrentText(), playerField->GetCurrentText().c_str());
    
	scene->LoadLevel(level, p);
    hud->LoadLevel(level);
    
    WindowResizeCallback(win_width, win_height);
}

void Exit(void *data)
{
    if (scene)
        delete scene;
    if (hud)
        delete hud;
    if (menu)
        delete menu;
    if (credits)
        delete credits;
    if (start)
        delete start;
    
    finished = true;
}

void LoadStartMenu(void *data)
{
    menu->PushMenu(start);
}

void LoadCreditsMenu(void *data)
{
    menu->PushMenu(credits);
}

void LoadHighScoresMenu(void *data)
{
    menu->PushMenu(hscores);
}

void CreateStartMenu()
{
    MenuItem **items = new MenuItem *[3];

    ipField = new TextField("SERVER IP: ", "127.0.0.1");
    playerField = new TextField("PLAYER: ", "1s");
    
    items[0] = ipField;
    items[1] = playerField;
    items[2] = new MenuItem("CONNECT", StartGame);
    
    start = new Menu(items, 3);
}

void CreateHighScoresMenu()
{
    MenuItem **items = new MenuItem *[11];
    
    items[0] = new MenuItem("HIGH SCORES:", NULL);
    items[1] = new MenuItem("", NULL);
    items[2] = new MenuItem("", NULL);
    items[3] = new MenuItem("", NULL);
    items[4] = new MenuItem("", NULL);
    items[5] = new MenuItem("", NULL);
    items[6] = new MenuItem("", NULL);
    items[7] = new MenuItem("", NULL);
    items[8] = new MenuItem("", NULL);
    
    hscores = new Menu(items, 9, 48);
}

void CreateCreditsMenu()
{
    MenuItem **items = new MenuItem *[9];
    
    items[0] = new MenuItem("DEVELOPMENT:", NULL);
    items[1] = new MenuItem("tom linford", NULL);
    items[2] = new MenuItem("ben-han sung", NULL);
    items[3] = new MenuItem("", NULL);
    items[4] = new MenuItem("SPECIAL THANKS TO:", NULL);
    items[5] = new MenuItem("michael lentine", NULL);
    items[6] = new MenuItem("jonathan su", NULL);
    items[7] = new MenuItem("bo zhu", NULL);
    items[8] = new MenuItem("saket patkar", NULL);
    
    credits = new Menu(items, 9, 48);
}

void CreateMainMenu()
{
    MenuItem **items = new MenuItem *[4];
    
    items[0] = new MenuItem("NEW GAME", LoadStartMenu);
    items[1] = new MenuItem("HIGH SCORES", LoadHighScoresMenu);
    items[2] = new MenuItem("CREDITS", LoadCreditsMenu);
    items[3] = new MenuItem("EXIT", Exit);
    
    menu = new Menu(items, 4);
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
int main(int argc, char *argv[])
{
	if (!glfwInit()) {
		cerr << "Failed to initialize glfw" << endl;
		exit(-1);
	}
	
	Sound::Init();

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

    CreateMainMenu();
    CreateStartMenu();
    CreateCreditsMenu();
    CreateHighScoresMenu();

    glfwSetCharCallback(CharCallback);    
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
        
        if (finished) {
            break;
        }
        
        if (gaming) {
            scene->Render();
            hud->Render();
            gaming = !scene->gameOver;
        }
        else {
            menu->Render();
        }
        
		glfwSwapBuffers();
	}
	glfwTerminate();
	return 0;
}