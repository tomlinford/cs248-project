#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <vector>
#include <stdlib.h>
#include <time.h>

#include "Program.h"

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

class ParticleCluster
{
public:
    ParticleCluster(glm::vec3 location);
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

class ParticleSystem
{
public:
    void Update();
    void AddCluster(glm::vec3 location);
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES);
    
private:
    std::vector<ParticleCluster> clusters;
};

