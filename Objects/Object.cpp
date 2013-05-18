#include "Object.h"

using namespace::glm;

Object::Object()
{
}

Object::Object(Model *m)
{
    assert (m != NULL);
    model = m;
}

Object::~Object()
{
    if (model)
        delete model;
};

// TODO: Implement
#pragma mark -
/** Determine if two bounding boxes overlap */
bool Object::Intersects(Object other)
{
    return false;
}

// TODO: Implement
#pragma mark -
/** Returns axis-aligned world-space bounds */
Bounds GetBounds()
{
}

void Object::Draw(const Program& p, const glm::mat4& viewProjection,
				 const glm::vec3& cameraPos, GLenum mode) const
{
	mat4 model = glm::translate(mat4(1), position);
    mat4 rotation = mat4_cast(orientation);
    model = model * rotation;
    mat4 mvp = viewProjection * model;
    
    p.Use();
    p.SetModel(model); // Needed for Phong shading
	p.SetMVP(mvp);
    
	Object::model->Draw(p, mode);
    
    p.Unuse();
}