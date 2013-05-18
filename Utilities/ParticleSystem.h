#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <vector>
#include <stdlib.h>
#include <time.h>

#include "Program.h"

/* A single particle */
class Particle
{
public:
    Particle(glm::vec3 l, glm::vec3 v, float s);
    glm::vec3 location;
    glm::vec3 velocity;
    
    float scale;
    float age;
    float lifetime;
    
    void Update();
    bool Valid() { return (age / lifetime) < 1.0; }
};

/** A particle cluster represents an individual group of particles
 forming a related effect. */
class ParticleCluster
{
public:
    ParticleCluster(glm::vec3 location, glm::vec3 color);
    //~ParticleCluster();
    
    void AddParticle(glm::vec3 location, glm::vec3 velocity, float scale);
    void Update();
    bool Valid() { return particles.size() > 0; }
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);

private:
    std::vector<Particle> particles;
    std::vector<glm::vec3> particleVertices;
    std::vector<glm::vec3> particleNormals;
    std::vector<float> particleIndices;
    glm::vec3 color;
};

/** A particle system is made up of a bunch of particle clusters, each
 of which represent a single effect in the scene. */
class ParticleSystem
{
public:
    void Update();
    void AddCluster(glm::vec3 location, glm::vec3 color);
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);
    
private:
    std::vector<ParticleCluster> clusters;
};

