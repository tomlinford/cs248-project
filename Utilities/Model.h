#pragma once

#include "../gl.h"

#include <glm/glm.hpp>

#include "Program.h"
#include "Buffer.h"
#include "Material.h"

/** Bounding box */
struct Bounds
{
    Bounds() {}
    
    /*     b4_______b3
     *    /|        /|
     *  f4_|_______f3|
     *  |  |      |  |
     *  |  |      |  |
     *  |  b1_____|__b2
     *  | /       | /
     *  f1________f2
     *
     *
     *  f3 = Upper (max X, max Y, max Z)
     *  b1 = Lower (min X, min Y, min Z)
     */
    Bounds(glm::vec3 l, glm::vec3 u) {
        b1 = l;
        b2 = glm::vec3(u.x, l.y, l.z);
        b3 = glm::vec3(u.x, u.y, l.z);
        b4 = glm::vec3(l.x, u.y, l.z);
        f1 = glm::vec3(l.x, l.y, u.z);
        f2 = glm::vec3(u.x, l.y, u.z);
        f3 = u;
        f4 = glm::vec3(l.x, u.y, u.z);
    }
    
    // Faces are defined in x-y plane
    glm::vec3 b1, b2, b3, b4; // Vertices for back face
    glm::vec3 f1, f2, f3, f4; // Vertices for front face
    
    Bounds& operator=(const Bounds& other) {
        b1 = other.b1;
        b2 = other.b2;
        b3 = other.b3;
        b4 = other.b4;
        f1 = other.f1;
        f2 = other.f2;
        f3 = other.f3;
        f4 = other.f4;
        return *this;
    }
};

class Model {
public:
	Model(const ModelBuffer& mb, Material mat, Bounds b);

	void Draw(const Program& p, GLenum mode = GL_TRIANGLES) const;
    
    // Model's bounds in model space
    Bounds bounds;

private:
	ModelBuffer modelBuffer;

	// Material of model
	Material mat;
};