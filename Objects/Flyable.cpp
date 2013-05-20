#include "Flyable.h"

using namespace::std;
using namespace::glm;

Flyable::Flyable(Model *model) : Object(model)
{
}

Flyable::Flyable(const string& filename) : Object(filename)
{
}

/** Draws the object */
void Flyable::Draw(const Program& p, const glm::mat4& viewProjection,
                   const glm::vec3& cameraPos, GLenum mode) const
{
    mat4 model = glm::translate(mat4(1), position);
    mat4 mvp = viewProjection * model;
    
    p.Use();
    p.SetModel(model); // Needed for Phong shading
	p.SetMVP(mvp);
    
	Object::model->Draw(p, mode);
    
    p.Unuse();
    
#ifdef DEBUG
    DrawAABB(p, viewProjection, cameraPos, mode);
#endif
}

Ship::Ship(Model *model) : Flyable(model)
{
}

Ship::Ship(const string& filename) : Flyable(filename)
{
}