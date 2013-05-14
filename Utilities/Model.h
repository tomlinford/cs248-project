#pragma once

#include <glm/glm.hpp>

#include "Program.h"
#include "Buffer.h"
#include "Material.h"

class Model {
public:
	Model(const ModelBuffer& mb, Material mat);

	void Draw(const Program& p, const glm::mat4& viewProjection,
		const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES) const;

private:
	ModelBuffer modelBuffer;

	// model matrix
	glm::mat4 model;

	// material of model
	Material mat;
};