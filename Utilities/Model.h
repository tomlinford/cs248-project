#pragma once

#include "../gl.h"

#include <glm/glm.hpp>

#include "Program.h"
#include "Buffer.h"
#include "Material.h"

class Model {
public:
	Model(const ModelBuffer& mb, Material mat);

	void Draw(const Program& p, GLenum mode = GL_TRIANGLES) const;

private:
	ModelBuffer modelBuffer;

	// Material of model
	Material mat;
};