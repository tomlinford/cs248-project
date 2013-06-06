#pragma once

#include "Object.h"
#include "ParticleSystem.h"

/* The Flyable class represents Objects that can
   move/fly. These objects have a speed and direction
   information in addition to the default properties.
   Bullets, ships, and asteroids should all fall under
   'Flyable'. */
class Flyable : public Object
{
public:
    Flyable(Model *model) : Object(model) {}
    Flyable(const string& filename) : Object(filename) {}
    
    /** Set speed. This should be in voxels per
     animation interval. */
    virtual float GetSpeed() { return speed; }
    virtual void SetSpeed(float s) { speed = s; }
    
    /** Set offset */
    virtual glm::vec2 GetOffset() { return offset; }
    virtual void SetOffset(glm::vec2 o) { offset = o; }
    
    /** Set time offset */
    virtual float GetTimeOffset() { return timeOffset; }
    virtual void SetTimeOffset(float to) { timeOffset = to; }
    
    /** Set direction */
    virtual glm::vec3 GetDirection() { return direction; }
    virtual void SetDirection(glm::vec3 dir);
    
    /** Set position - we override the Object's SetPosition
     method to add the offset to the position */
    virtual void SetPosition(glm::vec3 p);
    
protected:
    float speed;
    float timeOffset;
    glm::vec2 offset;
    glm::vec3 direction;
};

/* A subclass of Flyable representing a 
 missile */
class Missile : public Flyable
{
public:
    Missile(Model *model);
    Missile(const string& filename);
};

/* A subclass of the Flyable to represent the
   player;s ship. */
class Ship : public Flyable
{
public:
    Ship(Model *model);
    Ship(const string& filename);
    
    /** Override set color to update bullet cluster color
     as well */
    virtual void SetColor(glm::vec3 c);
    
    /** Adds a bullet */
    void AddBullet(glm::vec3 location, glm::vec3 velocity);
    
    /** Bullet cluster accessor */
    BulletCluster *GetBulletCluster() { return cluster; }
    
    /** Accessors for health */
    float GetHealth() { return health; }
    void SetHealth(float h) { health = h; }
    void AddDamage(float damage) { health -= damage; }
    
protected:
    float health;
    BulletCluster *cluster;
};
