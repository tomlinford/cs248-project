#include "ParticleSystem.h"

#define DEFAULT_LIFETIME 30
#define PARTICLES_PER_CLUSTER 100

using namespace::std;
using namespace::glm;

Particle::Particle(glm::vec3 l, glm::vec3 v, float s)
{
    location = l;
    velocity = v;
    scale = s;
    age = 0;
    lifetime = (float)rand() / RAND_MAX * DEFAULT_LIFETIME;
}


void Particle::Update()
{
    location += velocity;
    scale *= 0.8;
    age++;
}

ParticleCluster::ParticleCluster(glm::vec3 location, glm::vec3 c)
{
    color = c;
    
    srand(time(NULL));
    for (int i = 0; i < PARTICLES_PER_CLUSTER; i++)
    {
        vec3 velocity;
        velocity.x = rand();
        velocity.y = rand();
        velocity.z = rand();
        normalize(velocity);
        
        float scale = (float)rand() / RAND_MAX;
        
        AddParticle(location, velocity, scale);
    }
}

void ParticleCluster::AddParticle(glm::vec3 location, glm::vec3 velocity, float scale)
{
    particles.push_back(Particle(location, velocity, scale));
}

void ParticleCluster::Update()
{
    for (std::vector<Particle>::iterator it = particles.begin();
         it != particles.end();
         it++)
    {
        Particle particle = *it;
        if (!particle.Valid())
        {
            it = particles.erase(it);
            it--;
        }
        else {
            particle.Update();
        }
    }
}

void ParticleCluster::Draw(const Program& p, const glm::mat4& viewProjection,
                          const glm::vec3& cameraPos, GLenum mode)
{
    // TODO
}

void ParticleSystem::Update()
{
    for (std::vector<ParticleCluster>::iterator it = clusters.begin();
         it != clusters.end();
         it++)
    {
        ParticleCluster cluster = *it;
        if (!cluster.Valid()) {
            it = clusters.erase(it);
            it--;
        }
        else {
            cluster.Update();
        }
    }
}

void ParticleSystem::AddCluster(glm::vec3 location, glm::vec3 color)
{
    ParticleCluster cluster(location, color);
    clusters.push_back(cluster);
}

void ParticleSystem::Draw(const Program& p, const glm::mat4& viewProjection,
          const glm::vec3& cameraPos, GLenum mode)
{
    for (std::vector<ParticleCluster>::iterator it = clusters.begin();
         it != clusters.end();
         it++)
    {
        ParticleCluster cluster = *it;
        cluster.Draw(p, viewProjection, cameraPos, mode);
    }
}