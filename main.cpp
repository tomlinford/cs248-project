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

static Scene *scene;

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

void GLFWCALL WindowResizeCallback(int w, int h)
{
    float ratio = (float)w / h;
    
    // Update viewable area
    glViewport(0, 0, w, h);
    
    // Update projection matrix
    if (scene)
        scene->SetProjection(glm::perspective(75.0f,        // Field of view
                                              ratio,        // Aspect ratio
                                              0.1f,         // Near clipping plane
                                              100.0f));     // Far clipping plane
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
    if (argv[2][0] == '1')
        scene = new Scene(PLAYER1);
    else
        scene = new Scene(PLAYER2);
	Networking::Init(level, argv[1]);
    scene->LoadLevel(level);
    
	glfwSetWindowTitle("CS248 Project");
	glfwSetKeyCallback(KeyCallback);
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