#include "gl.h"

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "Utilities/Buffer.h"

using namespace std;
using namespace glm;

void GLFWCALL KeyCallback(int key, int action) {
	switch(key) {
        case GLFW_KEY_ESC:
            glfwCloseWindow();
            break;
	}
}

void Init() {
	if (!glfwInit()) {
		cerr << "Failed to initialize glfw" << endl;
		exit(-1);
	}

	// using opengl version 2.1
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

	if (!glfwOpenWindow(1024, 768, 5, 6, 5, 0, 24, 0, GLFW_WINDOW)) {
		cerr << "Failed to initialize glfw window" << endl;
		glfwTerminate();
		exit(-1);
	}

	glfwSetWindowTitle("CS248 Project");
	glfwSetKeyCallback(KeyCallback);
	
#ifndef __APPLE__
	glewInit();
#endif

	glEnable(GL_DEPTH_TEST | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(int argc, char *argv[]) {
	Init();

	// main render loop
	while(glfwGetWindowParam(GLFW_OPENED)) {
        Render();
		glfwSwapBuffers();
	}
	return 0;
}