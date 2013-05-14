#include "gl.h"

#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utilities/Buffer.h"
#include "Utilities/Model.h"
#include "Utilities/OBJFile.h"

using namespace std;
using namespace glm;

/** Window info */
static int winWidth, winHeight;

/** View projection info */
static mat4 projection;
static mat4 view;

static Program *flat;
static Model *ship;

void GLFWCALL KeyCallback(int key, int action) {
	switch(key) {
        case GLFW_KEY_ESC:
            glfwCloseWindow();
            break;
	}
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4 view = lookAt(vec3(0, 0, 5),   // Eye
                       vec3(0, 0, 0),   // Apple
                       vec3(0, 1, 0));  // Up
    
    ship->Draw(*flat, projection * view, vec3(0));
}


void GLFWCALL WindowResizeCallback(int w, int h)
{
    winWidth = w;
    winHeight = h;
    float ratio = (float)w / h;
    projection = glm::perspective(75.0f,        // Field of view
                                  ratio,        // Aspect ratio
                                  0.1f,         // Near clipping plane
                                  100.0f);      // Far clipping plane
    
    //perspective((float)45 - 1 * glfwGetMouseWheel(),
    //                              (float)4 / 3, (float)0.1, (float)100);
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void GLFWInit() {
	if (!glfwInit()) {
		cerr << "Failed to initialize glfw" << endl;
		exit(-1);
	}
    
#ifndef __APPLE__
	glewInit();
#else
    CGSetLocalEventsSuppressionInterval(0.0);
#endif

	// using opengl version 2.1
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

	if (!glfwOpenWindow(1024, 768, 5, 6, 5, 0, 24, 0, GLFW_WINDOW)) {
		cerr << "Failed to initialize glfw window" << endl;
		glfwTerminate();
		exit(-1);
	}

	glfwSetWindowTitle("CS248 Project");
	glfwSetKeyCallback(KeyCallback);
    glfwSetWindowSizeCallback(WindowResizeCallback);
}

void GLInit()
{
    glEnable(GL_DEPTH_TEST);    // Depth testing
    glEnable(GL_LINE_SMOOTH);   // Smooth lines
    glEnable(GL_MULTISAMPLE);   // Multisampling
    glEnable(GL_TEXTURE_2D);    // Texturing
}

void ObjectsInit()
{
    flat = new Program("Shaders/wirevertex.vert", "Shaders/wirefragment.frag");
    
    OBJFile *shipOBJ = new OBJFile("Models/ship.obj");
    ship = shipOBJ->GenModel();
}

int main(int argc, char *argv[]) {
    GLFWInit();
    GLInit();
	ObjectsInit();

	// Main render loop
	while(glfwGetWindowParam(GLFW_OPENED)) {
        Render();
		glfwSwapBuffers();
	}
	return 0;
}