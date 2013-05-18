#include "Model.h"

using namespace glm;

Model::Model(const ModelBuffer& mb, Material mat, Bounds b)
	: modelBuffer(mb), mat(mat)
{
    bounds = b;
}

void Model::Draw(const Program& p, GLenum mode) const
{
	modelBuffer.Draw(p, mode);
    
    /* Draw bounding box */
    
    p.SetUniform("illum", 0);
    p.SetUniform("baseColor", vec3(1.0));
}