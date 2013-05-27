#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <stdlib.h>
#include <time.h>

#include "Program.h"
#include "Buffer.h"
#include "Model.h"

/* A single particle */
class Particle
{
public:
    Particle(glm::vec3 l, glm::vec3 v, glm::vec3 f, float s);
    glm::vec3 location;
    glm::vec3 velocity;
    
    float scale;
    float age;
    float lifetime;
    glm::vec3 force;
    glm::quat orientation;
    
    void Update(float elapsedTime);
    bool Valid() { return (age / lifetime) < 1.0; }
};

/** A particle cluster represents an individual group of particles
 forming an explosion effect. */
class ParticleCluster
{
public:
    ParticleCluster() {}
    ParticleCluster(glm::vec3 location, glm::vec3 direction);
    //~ParticleCluster();
    
    void AddParticle(glm::vec3 location, glm::vec3 velocity, glm::vec3 force, float scale);
    virtual void Update(float elapsedTime);
    bool Valid() { return particles.size() > 0; }
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);

protected:
    std::vector<Particle> particles;
    std::vector<glm::vec3> particleVertices;
    std::vector<glm::vec3> particleNormals;
    std::vector<float> particleIndices;
    glm::vec3 color;
};

/** A fluid cluster represents an individual group of particles
 forming a fire/smoke effect. */
class FluidCluster : public ParticleCluster
{
public:
    FluidCluster(glm::vec3 location, glm::vec3 wind, glm::vec3 color);
    //~FluidCluster();
    
    virtual void Update(float elapsedTime);
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);
    
private:
    glm::vec3 location;
    glm::vec3 wind;
};

/** A particle system is made up of a bunch of particle clusters, each
 of which represent a single effect in the scene. */
class ParticleSystem
{
public:
	ParticleSystem() : lastTime(0) {}
    void Update(float elapsedTime);
    void AddExplosionCluster(glm::vec3 location, glm::vec3 color);
    void AddFluidCluster(glm::vec3 location, glm::vec3 wind, glm::vec3 color);
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);
    
private:
    std::vector<ParticleCluster> clusters;
    float lastTime;
};

