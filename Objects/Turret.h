#pragma once

#include "Object.h"
#include "ParticleSystem.h"

class Turret : public Object
{
public:
    Turret(Model *m);
    Turret(const string& filename);
    
    /** Score value */
    virtual int GetValue() { return 30; }
    
    /** Override set color to update bullet cluster color
         as well */
    virtual void SetColor(glm::vec3 c);
    
    /** Adds a bullet */
    void AddBullet(glm::vec3 location, glm::vec3 velocity);
    
    /** Bullet cluster accessor */
    BulletCluster *GetBulletCluster() { return cluster; }
    
private:
    BulletCluster *cluster;
};
