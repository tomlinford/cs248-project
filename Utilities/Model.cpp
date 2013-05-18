#include "Model.h"

using namespace glm;

Model::Model(const ModelBuffer& mb, Material mat)
	: modelBuffer(mb), mat(mat)
{
}

void Model::Draw(const Program& p, GLenum mode) const
{
	modelBuffer.Draw(p, mode);
}