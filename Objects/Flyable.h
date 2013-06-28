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
    ~Flyable() {}
    
    /** Score value */
    virtual int GetValue() { return 15; }
    
    /** Set speed. This should be in voxels per
     animation interval. */
    virtual float GetSpeed() { return speed; }
    virtual void SetSpeed(float s) { speed = s; }
    
    /** Set offset */
    virtual glm::vec2 GetOffset() { return offset; }
    virtual void SetOffset(glm::vec2 o) { offset = o; oldOffset = o; }
    
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
	glm::vec2 oldOffset;
    glm::vec3 direction;
};

/* A subclass of Flyable representing a 
 missile */
class Missile : public Flyable
{
public:
    Missile(Model *model);
    Missile(const string& filename);
    ~Missile() {}
    
    /** Score value */
    virtual int GetValue() { return 40; }
};

/* A subclass of the Flyable to represent the
   player;s ship. */
class Ship : public Flyable
{
public:
    Ship(Model *model);
    Ship(const string& filename);
    ~Ship();
    
    /** Score value */
    virtual int GetValue() { return 20; }
    
    /** Override set color to update bullet cluster color
     as well */
    virtual void SetColor(glm::vec3 c);
    
    /** Adds a bullet */
    void AddBullet(glm::vec3 location, glm::vec3 velocity);
    
    /** Get last fire time */
    float GetFiringRate() { return firingRate; }
    float GetLastFireTime() { return lastFireTime; }
    void SetLastFireTime(float f) { lastFireTime = f; }
    
    /** Bullet cluster accessor */
    BulletCluster *GetBulletCluster() { return cluster; }
    
protected:
    BulletCluster *cluster;
    float firingRate;
    float lastFireTime;
};

/* A subclass of the Ship to represent the
 ships that shoot missiles */
class MissileShip : public Ship
{
public:
    MissileShip(Model *model);
    MissileShip(const string& filename);
};

/* A subclass of the Ship to represent the
 ships that shoots lasers */
class LaserShip : public Ship
{
public:
    LaserShip(Model *model);
    LaserShip(const string& filename);
    ~LaserShip();
    
    virtual void SetColor(glm::vec3 c);
    
    /** Laser accessor */
    Laser *GetLaser() { return laser; }
    
protected:
    Laser *laser;
};
