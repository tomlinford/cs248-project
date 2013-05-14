#include "gl.h"

#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utilities/Buffer.h"
#include "Utilities/Model.h"

using namespace std;
using namespace glm;

static Program *flat;
static Model *triangle;

void GLFWCALL KeyCallback(int key, int action) {
	switch(key) {
        case GLFW_KEY_ESC:
            glfwCloseWindow();
            break;
	}
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4 projection = perspective((float)45 - 1 * glfwGetMouseWheel(),
                                  (float)4 / 3, (float)0.1, (float)100);
    mat4 view = lookAt(vec3(0, 0, 5), vec3(0), vec3(0, 1, 0));
    
    triangle->Draw(*flat, projection * view, vec3(0));
}

void Init() {
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
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

	if (!glfwOpenWindow(1024, 768, 5, 6, 5, 0, 24, 0, GLFW_WINDOW)) {
		cerr << "Failed to initialize glfw window" << endl;
		glfwTerminate();
		exit(-1);
	}

	glfwSetWindowTitle("CS248 Project");
	glfwSetKeyCallback(KeyCallback);

	glEnable(GL_DEPTH_TEST | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
    
    flat = new Program("Shaders/wirevertex.vert", "Shaders/wirefragment.frag");
}

int main(int argc, char *argv[]) {
	Init();

	vector<vec3> vertices;
	vertices.push_back(vec3(0)); vertices.push_back(vec3(1,0,0)); vertices.push_back(vec3(0,1,0));
	vector<size_t> indices;
	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	ArrayBuffer<vec3> ab(vertices);
	ElementArrayBuffer eab(indices);
	ModelBuffer mb(ab, eab);
    triangle = new Model(mb, Material());

	// main render loop
	while(glfwGetWindowParam(GLFW_OPENED)) {
        Render();
		glfwSwapBuffers();
	}
	return 0;
}