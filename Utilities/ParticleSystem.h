#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <mutex>

#include "Program.h"
#include "Buffer.h"
#include "Model.h"
#include "Object.h"

/* A single particle */
class Particle
{
public:
    /* Initializes particle with a given location, velocity
     force, and scale */
    Particle(glm::vec3 l, glm::vec3 v, glm::vec3 f, float s);
	Particle() {}
	~Particle() {}
    glm::vec3 location;
    glm::vec3 velocity;
    
    float scale;
    float age;
    float lifetime;
    glm::vec3 force;
    glm::quat orientation;
    
    glm::vec3 o1, o2, o3;
    
    void Update(float elapsedTime);
    bool Valid() { return (age / lifetime) < 1.0; }
};

/** A particle cluster represents an individual group of particles
 forming an explosion effect. */
class ParticleCluster
{
public:
    ParticleCluster() : deleteModel(false) {}
    ParticleCluster(glm::vec3 location, glm::vec3 color);
    ~ParticleCluster();
    
    void SetColor(glm::vec3 c) { color = c; }
    
    void AddParticle(glm::vec3 location, glm::vec3 velocity, glm::vec3 force, float scale);
    virtual bool Intersects(Object *object) { return false; }
    virtual void Update(float elapsedTime);
    virtual bool Valid() { return particles.size() > 0; }
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, DrawMode mode);

protected:
    std::mutex mutex;
    std::vector<Particle> particles;
    std::vector<glm::vec3> particleVertices;
    std::vector<float> particleIndices;
    glm::vec3 color;
    
    Model *model;
	bool deleteModel;
};

/* Represents a single long laser */
class Laser : public ParticleCluster
{
public:
    Laser(Object *o);
    ~Laser();
    
    virtual void Update(float elapsedTime);
    virtual bool Intersects(Object *object);
    virtual bool Valid();
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, DrawMode mode);
    void SetOwner(Object *o);
    
private:
    Object *owner;
    float lineWidth;
    glm::mat4 M;
};

/* Represents a lightning bolt. Generated using midpoint
 displacement algorithm, given the start and end points. */
class Bolt : public ParticleCluster
{
public:
    Bolt(glm::vec3 s, glm::vec3 e);
    virtual void Update(float elapsedTime);
    virtual bool Valid();
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                  const glm::vec3& cameraPos, DrawMode mode);
    
private:
    void GenKeyPoints(glm::vec3 &s, glm::vec3& e, float maxOffset, uint8_t depth);
};

/** A bullet cluster is a permanent cluster that belongs to
 an Object. The Object will add bullets to this cluster
 when desired. (The object is an "emitter" so to speak) */
class BulletCluster : public ParticleCluster
{
public:
    BulletCluster();
    
    void AddBullet(glm::vec3 l, glm::vec3 v);
    void SetOwner(Object *o);
    virtual bool Intersects(Object *object);
    virtual bool Valid() { return true; }
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, DrawMode mode);
    
private:
    Object *owner;
};

/** A particle system is made up of a bunch of particle clusters, each
 of which represent a single effect in the scene. */
class ParticleSystem
{
public:
	ParticleSystem() : lastTime(0) {}
    bool Intersects(Object *object);
    void Update(float elapsedTime);
    void AddBulletCluster(BulletCluster *cluster);
    void AddExplosionCluster(glm::vec3 location, glm::vec3 color);
    void AddBolt(glm::vec3 start, glm::vec3 end);
    void AddLaser(Laser *laser);
    void Clear();
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, DrawMode mode);
    
private:
    std::vector<ParticleCluster *> clusters;
    float lastTime;
};

