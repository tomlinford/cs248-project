#include "Flyable.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

#ifndef M_PI
#define M_PI 3.14159265359
#endif

using namespace::std;
using namespace::glm;

void Flyable::SetDirection(vec3 dir) {
    direction = normalize(dir);
    
    // 1. Find axis of rotation from default direction (0, 0, -1) to current direction
    // 2. Find angle of rotation along that axis
    // 3. Build quaternion
    vec3 z(0, 0, -1);
    vec3 axis = normalize(-cross(direction, z));;
    float cos_angle = dot(direction, z);
    float angle = acos(cos_angle) * 180 / M_PI;
    
    orientation = angleAxis(angle, axis);
    M = translate(glm::mat4(1), position) *
        mat4_cast(orientation) *
        glm::scale(mat4(1), vec3(scale));
}

void Flyable::SetPosition(vec3 p)
{
    vec3 x(1, 0, 0);
    vec3 y(0, 1, 0);
    vec3 x_dir = orientation * x;
    vec3 y_dir = orientation * y;
    
    position = p + offset.x * x_dir + offset.y * y_dir;
    M = translate(glm::mat4(1), position) *
        mat4_cast(orientation) *
        glm::scale(mat4(1), vec3(scale));
}

Missile::Missile(Model *model) : Flyable(model)
{
    scale = 0.1;
}

Missile::Missile(const string& filename) : Flyable(filename)
{
    scale = 0.1;
}

Ship::Ship(Model *model) : Flyable(model)
{
    health = 10;
}

Ship::Ship(const string& filename) : Flyable(filename)
{
    health = 10;
}