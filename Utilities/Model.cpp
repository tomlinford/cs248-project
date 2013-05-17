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

    p.Use();
    p.SetModel(model); // Needed for Phong shading
	p.SetMVP(mvp);
    
	modelBuffer.Draw(p, mode);
    
    p.Unuse();
}