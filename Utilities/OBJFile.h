#pragma once

#include "../gl.h"

#include <vector>
#include <glm/glm.hpp>

// ignore me
struct VertexIndex {
    GLuint v, t, n;
};

/** Represents a parsed OBJ file. All members a easily accessible to
    clients of the struct. */
class OBJFile {
public:
    OBJFile(const char *filename);
    ~OBJFile() {}

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textures;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;

private:
	

    

    void readFile(std::vector<glm::vec3>& vertCoords, std::vector<glm::vec2>& texCoords,
                  std::vector<glm::vec3>& normals, std::vector<VertexIndex>&
                  vertexIndices, float& max, const char *filename);
};