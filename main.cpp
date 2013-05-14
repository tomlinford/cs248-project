#include "gl.h"

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utilities/Buffer.h"
#include "Utilities/Model.h"

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

int main(int argc, char *argv[]) {
	Init();

	vector<vec3> vertices;
	vertices.push_back(vec3(0)); vertices.push_back(vec3(1,0,0)); vertices.push_back(vec3(0,1,0));
	vector<size_t> indices;
	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	ArrayBuffer<vec3> ab(vertices);
	ElementArrayBuffer eab(indices);
	ModelBuffer mb(ab, eab);
	Model triangle(mb, Material());

	Program p("Shaders/wirevertex.glsl", "Shaders/wirefragment.glsl");
	

	// main render loop
	while(glfwGetWindowParam(GLFW_OPENED)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mat4 projection = perspective((float)45 - 1 * glfwGetMouseWheel(),
                                      (float)4 / 3, (float)0.1, (float)100);
        mat4 view = lookAt(vec3(0, 0, 5), vec3(0), vec3(0, 1, 0));
        
        triangle.Draw(p, projection * view, vec3(0));
		glfwSwapBuffers();
	}
	return 0;
}