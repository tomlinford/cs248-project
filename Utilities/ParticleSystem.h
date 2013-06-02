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
    ParticleCluster() {}
    ParticleCluster(glm::vec3 location, glm::vec3 color);
    //~ParticleCluster();
    
    void SetColor(glm::vec3 c) { color = c; }
    
    void AddParticle(glm::vec3 location, glm::vec3 velocity, glm::vec3 force, float scale);
    virtual bool Intersects(Object *object) { return false; }
    virtual void Update(float elapsedTime);
    virtual bool Valid() { return particles.size() > 0; }
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);

protected:
    std::mutex mutex;
    std::vector<Particle> particles;
    std::vector<glm::vec3> particleVertices;
    std::vector<float> particleIndices;
    glm::vec3 color;
};

/** A bullet cluster is a permanent cluster that belongs to
 an Object. The Object will add bullets to this cluster
 when desired. (The object is an "emitter" so to speak) */
class BulletCluster : public ParticleCluster
{
public:
    void AddBullet(glm::vec3 l, glm::vec3 v);
    virtual bool Intersects(Object *object);
    virtual bool Valid() { return true; }
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);
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
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);
    
private:
    std::vector<ParticleCluster *> clusters;
    float lastTime;
};

