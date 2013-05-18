#pragma once

#include "Object.h"

class Flyable : public Object
{
public:
    Flyable(Model *model);
    
    /** Set speed. This should be in voxels per
     animation interval. */
    float GetSpeed() { return speed; }
    void SetSpeed(float s) { speed = s; }
    
    /** Set direction */
    glm::vec3 GetDirection() { return direction; }
    void SetDirection(glm::vec3 dir) { direction = dir; }
    
protected:
    float speed;
    glm::vec3 direction;
};

class Ship : public Flyable
{
public:
    Ship(Model *model);
    
protected:
    float health;
};
