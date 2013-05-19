#pragma once

#include "gl.h"

#include <glm/glm.hpp>

#include "Buffer.h"
#include "Texture.h"

class Terrain {
public:
	Terrain(GLfloat *terrainMap, size_t size);
	~Terrain();

	void Draw(const glm::mat4& viewProjection);

private:
	ModelBuffer *triangleMB;
	ModelBuffer *lineMB;
	Program p;
	Texture *heightField;
};