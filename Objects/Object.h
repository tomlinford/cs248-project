#pragma once

#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/** Bounding box */
struct Bounds
{
    glm::vec3 lower; // Lower corner in model space
    glm::vec3 upper; // Upper (opposing) corner in model space
};

class Object
{
    virtual bool Intersects(Object other);
    
protected:
    Model *model;
    Bounds bounds;
    glm::vec3 position;
    glm::quat orientation;
};
