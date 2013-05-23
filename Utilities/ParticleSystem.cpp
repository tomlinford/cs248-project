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
        vec3 v1 = particle.location + vec3(0.1, 0, 0);
        vec3 v2 = particle.location + vec3(0.0, sqrt(0.3), 0.0);
        vec3 v3 = particle.location + vec3(-0.1, 0, 0);
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        indices.push_back(index++);
        indices.push_back(index++);
        indices.push_back(index++);
    }
    
    // Create model
	ArrayBuffer<vec3> vertexBuf(vertices);
	ElementArrayBuffer elements(indices);
	Model *model = new Model(ModelBuffer(vertexBuf, elements), Material(), Bounds());
    
    mat4 M = mat4(1);
    mat4 MVP = viewProjection * M;
    
    p.SetUniform("baseColor", vec3(1.0f, 0.0f, 1.0f));
    p.SetModel(M); // Needed for Phong shading
	p.SetMVP(MVP);
    
    p.SetUniform("illum", 0);
	model->Draw(p, GL_LINE_LOOP);
    
	p.SetUniform("illum", 1);
	model->Draw(p, GL_TRIANGLES);
    
    delete model;
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