#include "gl.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <sstream>
#include <glm/glm.hpp>
#include <time.h>
#include <fmod.hpp>

#include "Scene.h"
#include "HUD.h"
#include "Menu.h"
#include "Level.h"
#include "Networking.h"
#include "Sound.h"

#include "Leap.h"
#include "LeapListener.h"

// memory leak detection
//#include <vld.h>

using namespace std;
using namespace glm;
using namespace Leap;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

static Controller controller;
static LeapListener listener;

static Menu *menu, *start, *credits, *hscores, *nextLevel, *connect, *hscoreEntry;
static Scene *scene;
static HUD *hud;
static Player p;
static int levelNum = 0;

TextField *ipField;
TextField *playerField;
TextField *p1Field;
TextField *p2Field;
MenuItem *scoreField;
MenuItem *totalScoreField;
MenuItem **hscoreItems;

std::vector<string> scores;

bool netInit;
bool finished;
bool gaming;
bool menuKeyDown;

static float win_width, win_height;

template<typename T>
static string toString(T t) {
	stringstream s;
	s.precision(2);
	s << t;
	return s.str();
}

void moveShip()
{
    const Frame frame = controller.frame();
    if (frame.hands().empty() || !scene->level)
        return;
    
    const Hand leftHand = frame.hands().leftmost();
    const Hand rightHand = frame.hands().rightmost();
    
    // Alternative flight control
    /* const Vector normal = hand.palmNormal();
    const Vector direction = hand.direction();
    
    // Calculate the hand's pitch, roll, and yaw angles
    std::cout << "Hand pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
    << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
    << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl << std::endl;
    
    scene->keyUp = (direction.pitch() * RAD_TO_DEG > 5.0f);
    scene->keyDown = (direction.pitch() * RAD_TO_DEG < -5.0f);
    scene->keyLeft = ((normal.roll() * RAD_TO_DEG) > 15.0f);
    scene->keyRight = ((normal.roll() * RAD_TO_DEG) < -15.0f);*/
    
    Vector position = leftHand.palmPosition();
    vec2 shipOffset = vec2(0.06 * (position.x + 100), 0.06 * (position.y - 120));
	scene->level->ship->SetOffset(shipOffset);
    
    if (frame.hands().count() > 1) {
        hud->reticleVisible = true;
        position = rightHand.palmPosition();
        vec2 reticleOffset = vec2(5.0 * (position.x - 100), -5.0 * (position.y - 120));
        hud->SetReticlePosition(reticleOffset);
        
        if (rightHand.fingers().count() > 3) {
            reticleOffset.y *= -1;
            scene->HandleHand(reticleOffset);
        }
	}
    else {
        hud->reticleVisible = false;
    }
}

void GLFWCALL KeyCallback(int key, int action)
{
	if (!menuKeyDown && scene && gaming && p == PLAYER1) {
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
            default:
                break;
		}
        Networking::KeyAction(key, action, scene->GetShipOffset());
	}
    else {
        menuKeyDown = (action == GLFW_PRESS);
        if (menu) {
            menu->HandleKey(key, action);
        }
    }
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
	if (win_width > 0 && win_height > 0 && scene && gaming)
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
        hud->Resize(w, h);
    }
    if (menu) {
        menu->SetDimensions(w, h);
        start->SetDimensions(w, h);
        credits->SetDimensions(w, h);
        hscores->SetDimensions(w, h);
        hscoreEntry->SetDimensions(w, h);
        nextLevel->SetDimensions(w, h);
        connect->SetDimensions(w, h);
    }

	// Update global
	win_width = w;
	win_height = h;
}

void StartGame(void *data)
{
	// test to make sure server is alive
	if (!Networking::PingServer(ipField->GetCurrentText()))
		return;

    gaming = true;

	string playerString = playerField->GetCurrentText();
    
    // Choose player based on user input in menu
	switch (playerString[0]) {
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
	hud->Reset(levelNum);
    
    // Stream new level from server
    // Tom: Hook into difficulty levels here?
    Level *level = new Level(levelNum);
    Networking::Init(scene, level, ipField->GetCurrentText(), playerString.c_str(), levelNum);
    
	scene->LoadLevel(level, p);
    hud->LoadScene(scene);
    
    // Update Scene textures/FBO
    WindowResizeCallback(win_width, win_height);
    
    // Update menus
    start->SetSelectionActive(false);
    nextLevel->SetSelectionActive(false);
}

void Exit(void *data)
{
    if (scene)
        delete scene;
    if (hud)
        delete hud;
    if (menu) {
        delete menu;
        delete credits;
        delete start;
        delete hscores;
        delete hscoreEntry;
        delete nextLevel;
        delete connect;
    }
    
    finished = true;
}

void LoadMainMenu(void *data)
{
    while (menu->GetCurrentMenu() != menu)
        menu->PopSubMenu();
}

void UpdateHighScores(void *data)
{
    string entry = toString(scene->score);
    entry.append("         |         ");
    entry.append(p1Field->GetCurrentText());
    entry.append("         |         ");
    entry.append(p2Field->GetCurrentText());
    
    scores.push_back(entry);
    scene->score = 0;
    
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (i < scores.size())
            hscoreItems[i + 1]->label = scores[i];
        else
            hscoreItems[i + 1]->label = "";
    }
    
    hscoreEntry->PopMenu();
    LoadMainMenu(NULL);
}

void LoadStartMenu(void *data)
{
    menu->GetCurrentMenu()->PushMenu(start);
}

void LoadConnectMenu(void *data)
{
    menu->GetCurrentMenu()->PushMenu(connect);
}

void LoadCreditsMenu(void *data)
{
    menu->GetCurrentMenu()->PushMenu(credits);
}

void LoadHighScoresMenu(void *data)
{
    menu->GetCurrentMenu()->PushMenu(hscores);
}

void LoadHscoreEntryMenu(void *data)
{
    menu->GetCurrentMenu()->PushMenu(hscoreEntry);
}

void LoadNextLevelMenu(void *data)
{
    scoreField->label = "" + toString(scene->score);
    totalScoreField->label = "" + toString(scene->totalScore);
    
    Menu *top = menu->GetCurrentMenu();
    if (top != nextLevel)
        top->PushMenu(nextLevel);
}

void CreateHscoreEntryMenu()
{
    MenuItem **items = new MenuItem *[6];
    
    p1Field = new TextField("PLAYER1: ", "NAME");
    p2Field = new TextField("PLAYER2: ", "NAME");
    
    items[0] = new MenuItem("HIGH SCORE", NULL);
    items[1] = new MenuItem("", NULL);
    items[2] = p1Field;
    items[3] = p2Field;
    items[4] = new MenuItem("", NULL);
    items[5] = new MenuItem("CONTINUE", UpdateHighScores);
    
    hscoreEntry = new Menu(items, 6, 48);
}

void CreateNextLevelMenu()
{
    MenuItem **items = new MenuItem *[8];
    
    scoreField = new MenuItem("", NULL);
    totalScoreField = new MenuItem("", NULL);
    
    items[0] = new MenuItem("LEVEL COMPLETED", NULL);
    items[1] = new MenuItem("", NULL);
    items[2] = new MenuItem("LEVEL SCORE:", NULL);
    items[3] = scoreField;
    items[4] = new MenuItem("TOTAL SCORE:", NULL);
    items[5] = totalScoreField;
    items[6] = new MenuItem("", NULL);
    items[7] = new MenuItem("CONTINUE", StartGame);
    
    nextLevel = new Menu(items, 8, 48);
}

void CreateStartMenu()
{
    MenuItem **items = new MenuItem *[3];

    ipField = new TextField("SERVER IP: ", "127.0.0.1");
    playerField = new TextField("PLAYER: ", "1S");
    
    items[0] = ipField;
    items[1] = playerField;
    items[2] = new MenuItem("CONNECT", StartGame);
    
    start = new Menu(items, 3);
}

void CreateConnectMenu()
{
    MenuItem **items = new MenuItem *[3];
    
    items[0] = new MenuItem("CONNECTING...", NULL);
    items[1] = new MenuItem("", NULL);
    items[2] = new MenuItem("CANCEL", StartGame);
    
    connect = new Menu(items, 3);
}

void CreateHighScoresMenu()
{
    hscoreItems = new MenuItem *[9];
    
    hscoreItems[0] = new MenuItem("HIGH SCORES:", NULL);
    hscoreItems[1] = new MenuItem("", NULL);
    hscoreItems[2] = new MenuItem("", NULL);
    hscoreItems[3] = new MenuItem("", NULL);
    hscoreItems[4] = new MenuItem("", NULL);
    hscoreItems[5] = new MenuItem("", NULL);
    hscoreItems[6] = new MenuItem("", NULL);
    hscoreItems[7] = new MenuItem("", NULL);
    hscoreItems[8] = new MenuItem("", NULL);
    
    hscores = new Menu(hscoreItems, 9, 48);
}

void CreateCreditsMenu()
{
    MenuItem **items = new MenuItem *[9];
    
    items[0] = new MenuItem("DEVELOPERS:", NULL);
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
    MenuItem **items = new MenuItem *[6];
    
    items[0] = new MenuItem("GAME OF DRONES", NULL);
    items[1] = new MenuItem("", NULL);
    items[2] = new MenuItem("NEW GAME", LoadStartMenu);
    items[3] = new MenuItem("HIGH SCORES", LoadHighScoresMenu);
    items[4] = new MenuItem("CREDITS", LoadCreditsMenu);
    items[5] = new MenuItem("EXIT", Exit);
    
    menu = new Menu(items, 6);
    menu->enlargeTitle = true;
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
int main(int argc, char *argv[])
{
	if (!glfwInit()) {
		cerr << "Failed to initialize glfw" << endl;
		exit(-1);
	}
	
	Sound::Init();
    
    controller.addListener(listener);

	// using opengl version 2.1
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

#ifdef __APPLE__
	CGSetLocalEventsSuppressionInterval(0.0);
#endif

	if (!glfwOpenWindow(1280, 720, 5, 6, 5, 0, 24, 0, GLFW_WINDOW)) {
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
    CreateNextLevelMenu();
    CreateHighScoresMenu();
    CreateHscoreEntryMenu();
    CreateConnectMenu();

    glfwDisable(GLFW_MOUSE_CURSOR);
    
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
			//cout << nbFrames << " fps" << endl;
			nbFrames = 0;
			lastTime += 1.0;
		}
        
        if (finished) {
            break;
        }
        
        if (gaming) {
            moveShip();
            scene->Render();
            hud->Render();
            gaming = !scene->gameOver;
            if (scene->gameOver && scene->levelOver) {
				levelNum++;
                LoadNextLevelMenu(NULL);
            }
			else if (scene->gameOver) {
				levelNum = 0;
                nextLevel->PopMenu();
                LoadHscoreEntryMenu(NULL);
			}
        }
        else {
			if (menu->GetCurrentMenu() == menu) {
				levelNum = 0;
				if (scene) scene->totalScore = 0;
			}
            menu->Render();
        }
        
		glfwSwapBuffers();
	}
	glfwTerminate();
    controller.removeListener(listener);
	return 0;
}