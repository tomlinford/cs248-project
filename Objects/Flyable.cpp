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
    // What is wrong with this argh:
    // Trying to form a orientation quaternion that points the ship
    // in the direction vector.
    // 1. Find axis of rotation from default (0, 0, 1) to direction
    // 2. Find angle of rotation along that axis
    // 3. Build quaternion
    vec3 axis = normalize(cross(vec3(0, 0, -1), direction));
    float angle = acos(dot(vec3(0, 0, 1), normalize(direction)));
    orientation = normalize(fquat(angle, axis));
    
    mat4 model = glm::translate(mat4(1), position) * mat4_cast(orientation);
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