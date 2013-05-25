#include "gl.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif
#include <glm/glm.hpp>
#include <time.h>

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
}

void GLFWCALL MouseCallback(int x, int y) {
    // This gets called once before the window has been
    // initialized; the if block makes sure we don't
    // preset theta and phi to junk when that happens
    if (win_width > 0 && win_height > 0)
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

void GLFWCALL WindowResizeCallback(int w, int h)
{
    // Update viewable area
    glViewport(0, 0, w, h);
    
    // Update projection matrix
    float ratio = (float)w / h;
    if (scene)
        scene->SetProjection(glm::perspective(75.0f,        // Field of view
                                              ratio,        // Aspect ratio
                                              0.1f,         // Near clipping plane
                                              300.0f));     // Far clipping plane
    
    // Update global
    win_width = w;
    win_height = h;
    glfwSetMousePos(win_width / 2, win_height / 2);
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
    
#ifndef __APPLE__
	glewInit();
#endif
    
    // TODO: Stream levels from server
    Level *level = new Level();
    level->ship = new Ship("Models/ship.obj");
    level->ship->SetColor(vec3(0.0, 0.9, 0.0));
    
    // Choose player
    if (argv[2][0] == '1') {
        scene = new Scene(PLAYER1);
        glfwSetKeyCallback(KeyCallback);
    }
    else {
        scene = new Scene(PLAYER2);
        glfwSetMousePosCallback(MouseCallback);
    }
	Networking::Init(level, argv[1]);
    scene->LoadLevel(level);
    
	glfwSetWindowTitle("CS248 Project");
    glfwSetWindowSizeCallback(WindowResizeCallback);
    
    glEnable(GL_DEPTH_TEST);    // Depth testing
    glEnable(GL_LINE_SMOOTH);   // Smooth lines
    glEnable(GL_MULTISAMPLE);   // Multisampling
    glEnable(GL_TEXTURE_2D);    // Texturing
    
    // Seed random
    srand(time(NULL));

	// Main render loop
	while(glfwGetWindowParam(GLFW_OPENED)) {
        scene->Update();
        scene->Render();
		glfwSwapBuffers();
	}
	return 0;
}