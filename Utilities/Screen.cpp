#include "Screen.h"

using namespace::std;
using namespace::glm;

const static GLfloat QUAD_COORDS[] = {
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
};

const static GLuint QUAD_TEX[] = {
    0, 0,
    0, 1,
    1, 1,
    1, 0
};

const static GLubyte QUAD_INDICES[] = {
    0, 2, 1,
    2, 0, 3
};

Screen::Screen()
{
    vector<vec3> vertices;
    for (int i = 0; i < 12; i += 3) {
        vertices.push_back(vec3(QUAD_COORDS[i],
                                QUAD_COORDS[i + 1],
                                QUAD_COORDS[i + 2]));
    }
    
    vector<vec2> tex;
    for (int i = 0; i < 8; i += 2) {
        tex.push_back(vec2(QUAD_TEX[i],QUAD_TEX[i + 1]));
    }
    
	vector<size_t> indices;
	for (int i = 0; i < 6; i++) {
        indices.push_back(QUAD_INDICES[i]);
    }
	
	ArrayBuffer<vec3> abv(vertices);
    ArrayBuffer<vec2> abt(tex);
	ElementArrayBuffer eab(indices);
	ModelBuffer mb(abv, abt, eab);
    Bounds b(vec3(-1, -1, 0), vec3(1, 1, 0));
    quad = new Model(mb, Material(), b);
}

Screen::~Screen()
{
    delete quad;
}

void Screen::Draw(Program& p)
{
    p.Use();
    
    quad->Draw(p, GL_TRIANGLES);
    
    p.Unuse();
}