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
    virtual float GetSpeed() { return speed; }
    virtual void SetSpeed(float s) { speed = s; }
    
    /** Set offset */
    virtual glm::vec2 GetOffset() { return offset; }
    virtual void SetOffset(glm::vec2 o) { offset = o; }
    
    /** Set direction */
    virtual glm::vec3 GetDirection() { return direction; }
    virtual void SetDirection(glm::vec3 dir);
    
    /** Set position - we override the Object's SetPosition
     method to add the offset to the position */
    virtual void SetPosition(glm::vec3 p);
    
    /* Conversion - Euler angles to quaternion */
    glm::quat EulerAnglesToQuat(glm::vec3 angles) const;
    
protected:
    float speed;
    glm::vec2 offset;
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
