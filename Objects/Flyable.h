#pragma once

#include "Object.h"

class Flyable : public Object
{
private:
    float speed;
    glm::vec3 direction;
};
