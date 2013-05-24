#include "ParticleSystem.h"

#define MAX_LIFETIME 300
#define PARTICLES_PER_CLUSTER 80

using namespace::std;
using namespace::glm;

float rand(float min, float max) {
    return min + (float)random() / RAND_MAX * (max - min);
}

Particle::Particle(glm::vec3 l, glm::vec3 v, float s)
{
    location = l;
    velocity = v;
    scale = s;
    
    // Random lifetime up to MAX
    age = 0;
    lifetime = rand(0, MAX_LIFETIME);
    
    // Random orientation
    orientation = angleAxis(rand(0, 360), vec3(0, 0, 1)) *
    angleAxis(rand(0, 360), vec3(0, 1, 0)) *
    angleAxis(rand(0, 360), vec3(1, 0, 0));
}


void Particle::Update()
{
    location += velocity;
    velocity *= 0.95;
    velocity.y += -0.0001 * age;
    scale *= 0.95;
    age += 1;
}

ParticleCluster::ParticleCluster(glm::vec3 location, glm::vec3 c)
{
    color = c;
    
    srand(time(NULL));
    for (int i = 0; i < PARTICLES_PER_CLUSTER; i++)
    {
        vec3 velocity(rand(-0.1, 0.1),
                      rand(-0.1, 0.1),
                      rand(-0.1, 0.1));
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
    for (int i = 0; i < particles.size(); i++) {
        Particle &particle = particles[i];
        if (!particle.Valid())
        {
            particles.erase(particles.begin() + i);
            i--;
        }
        else {
            particle.Update();
        }
    }
}

// TODO: Generate particle buffer
void ParticleCluster::Draw(const Program& p, const glm::mat4& viewProjection,
                          const glm::vec3& cameraPos, GLenum mode)
{
    vector<vec3> vertices;
	vector<size_t> indices;
    
    // Add triangle for each particle
    int index = 0;
	for (std::vector<Particle>::iterator it = particles.begin();
         it != particles.end();
         it++)
    {
        Particle particle = *it;
        
        // Orient vertices
        vec3 o1 = particle.orientation * vec3(0.2, 0.0, 0.0);
        vec3 o2 = particle.orientation * vec3(0.0, sqrt(3.0) / 5, 0);
        vec3 o3 = particle.orientation * vec3(-0.2, 0.0, 0.0);
        
        vec3 v1 = particle.location + particle.scale * o1;
        vec3 v2 = particle.location + particle.scale * o2;
        vec3 v3 = particle.location + particle.scale * o3;
        
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        
        indices.push_back(index++);
        indices.push_back(index++);
        indices.push_back(index++);
    }
    
    cout << index / 3 << " particles drawn" << endl;
    
    ArrayBuffer<vec3> ab(vertices);
    ElementArrayBuffer eab(indices);
    Model model(ModelBuffer(ab, eab), Material(), Bounds());
    
    mat4 M = mat4(1);
    mat4 MVP = viewProjection * M;
    
    p.SetUniform("baseColor", color);
    p.SetModel(M); // Needed for Phong shading
    p.SetMVP(MVP);
    
    p.SetUniform("illum", 1);
    model.Draw(p, GL_TRIANGLES);
    
    p.SetUniform("baseColor", color);
	p.SetUniform("illum", 0);
	model.Draw(p, GL_LINE_LOOP);
    
    model.Delete();
}

void ParticleSystem::Update()
{
    for (std::vector<ParticleCluster>::iterator it = clusters.begin();
         it != clusters.end();
         it++)
    {
        ParticleCluster &cluster = *it;
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