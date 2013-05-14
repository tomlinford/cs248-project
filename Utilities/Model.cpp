#include "Model.h"

using namespace glm;

Model::Model(const ModelBuffer& mb, Material mat)
	: modelBuffer(mb), model(mat4(1)), mat(mat)
{
}

void Model::Draw(const Program& p, const glm::mat4& viewProjection,
				 const glm::vec3& cameraPos, GLenum mode) const {
	p.Use();
	mat4 mvp = viewProjection * model;

	modelBuffer.Draw(p, mode);
}