#include "Flyable.h"
#include <glm/gtx/quaternion.hpp>

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
                   const glm::vec3& cameraPos, GLenum mode)
{
    // What am I missing here ARGH
    mat4 x = glm::rotate(mat4(1), direction.x, vec3(0, 0, 1));
    mat4 y = glm::rotate(mat4(1), direction.y, vec3(1, 0, 0));
    mat4 z = glm::rotate(mat4(1), direction.z, vec3(0, 0, 1));
    mat4 model = x * y * z * glm::translate(mat4(1), position);
    mat4 mvp = viewProjection * model;
    
    cout << "Direction is " << direction.x << ", " << direction.y << ", " << direction.z << ")" << endl;
    
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