#pragma once

#include "Object.h"

/* The Flyable class represents Objects that can
   move/fly. These objects have a speed and direction
   information in addition to the default properties.
   Bullets, ships, and asteroids should all fall under
   'Flyable'. */
class Flyable : public Object
{
public:
    Flyable(Model *model);
    Flyable(const string& filename);
    
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

/* A subclass of the Flyable to represent the
   player;s ship. */
class Ship : public Flyable
{
public:
    Ship(Model *model);
    Ship(const string& filename);
    
    /** Accessors for health */
    float GetHealth() { return health; }
    void SetHealth(float h) { health = h; }
    
protected:
    float health;
};
