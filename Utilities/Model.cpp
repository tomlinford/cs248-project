#include "Model.h"

using namespace glm;

Model::Model(const ModelBuffer& mb, Material mat)
	: modelBuffer(mb), model(mat4(1)), mat(mat)
{
}

void Model::Draw(const Program& p, const glm::mat4& viewProjection,
				 const glm::vec3& cameraPos, GLenum mode) const
{
	mat4 mvp = viewProjection * model;

	vec4 pos = mvp * vec4(1);

    p.Use();
	GLenum err = glGetError();
	p.SetMVP(mvp);
	modelBuffer.Draw(p, mode);
    p.Unuse();
}