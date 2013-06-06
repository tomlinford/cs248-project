#include "ParticleSystem.h"

#define MAX_VELOCITY 5
#define MAX_LIFETIME 10
#define PARTICLES_PER_CLUSTER 80

using namespace::std;
using namespace::glm;

float rand(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

Particle::Particle(glm::vec3 l, glm::vec3 v, glm::vec3 f, float s)
{
    location = l;
    velocity = v;
    force = f;
    scale = s;
    
    // Random lifetime up to MAX
    age = 0;
    lifetime = rand(0, MAX_LIFETIME);
    
    // Random orientation
    orientation = angleAxis(rand(0, 360), vec3(0, 0, 1)) *
    angleAxis(rand(0, 360), vec3(0, 1, 0)) *
    angleAxis(rand(0, 360), vec3(1, 0, 0));
    
    // Orient vertices
    o1 = orientation * vec3(0.2, 0.0, 0.0);
    o2 = orientation * vec3(0.0, sqrt(3.0) / 5, 0);
    o3 = orientation * vec3(-0.2, 0.0, 0.0);
}

void Particle::Update(float elapsedTime)
{
    location += velocity * elapsedTime;
    velocity += force * elapsedTime;
    age += elapsedTime;
    scale *= (MAX_LIFETIME - age) / MAX_LIFETIME;
}

ParticleCluster::ParticleCluster(glm::vec3 location, glm::vec3 c)
{
    color = c;
    model = NULL;
    
    for (int i = 0; i < PARTICLES_PER_CLUSTER; i++)
    {
        vec3 velocity(rand(-MAX_VELOCITY, MAX_VELOCITY),
                      rand(-MAX_VELOCITY, MAX_VELOCITY),
                      rand(-MAX_VELOCITY, MAX_VELOCITY));
        normalize(velocity);
        
        float scale = (float)rand() / RAND_MAX;
        
        AddParticle(location, velocity, vec3(0, -MAX_VELOCITY, 0), scale);
    }
}

void ParticleCluster::AddParticle(glm::vec3 location, glm::vec3 velocity, glm::vec3 force, float scale)
{
    particles.push_back(Particle(location, velocity, force, scale));
}

void ParticleCluster::Update(float elapsedTime)
{
    for (int i = 0; i < particles.size(); i++) {
        Particle &particle = particles[i];
        if (!particle.Valid())
        {
            particles.erase(particles.begin() + i);
            i--;
        }
        else {
            particle.Update(elapsedTime);
        }
    }
    
    if (model) {
        //model->Delete();
        delete model;
        model = NULL;
    }
}

// Generates and draws particle buffer
void ParticleCluster::Draw(const Program& p, const glm::mat4& viewProjection,
                          const glm::vec3& cameraPos, bool glowMap)
{
    if (particles.size() == 0)
        return;
    
    if (!model) {
        // pre allocate space for vectors
        vector<vec3> vertices(particles.size() * 3);
        
        // add triangle for each particle
        for (size_t i = 0; i < particles.size(); i++) {
            Particle &particle = particles[i];
            
            vertices[i * 3] = particle.location + particle.scale * particle.o1;
            vertices[i * 3 + 1] = particle.location + particle.scale * particle.o2;
            vertices[i * 3 + 2] = particle.location + particle.scale * particle.o3;
        }
        
        ArrayBuffer<vec3> ab(vertices);
        
        model = new Model(ModelBuffer(ab, vertices.size() / 3), Material(), Bounds());
    }
    
    mat4 M = mat4(1);
    mat4 MVP = viewProjection * M;
    
    p.SetModel(M); // Needed for Phong shading
    p.SetMVP(MVP);
    
    if (!glowMap) {
        p.SetUniform("baseColor", color);
        p.SetUniform("illum", 1);
        model->Draw(p, GL_TRIANGLES);
    }
    
	// color has already been set, will have the same color as above
    p.SetUniform("baseColor", color);
	p.SetUniform("illum", 0);
	model->Draw(p, GL_LINE_LOOP);
}

Bolt::Bolt(glm::vec3 s, glm::vec3 e) {
    color = vec3(1.0);
    model = NULL;
    
    particles.push_back(Particle(s, vec3(0), vec3(0), 1.0));
    GenKeyPoints(s, e, 5.0, 10);
    particles.push_back(Particle(e, vec3(0), vec3(0), 1.0));
}

void Bolt::GenKeyPoints(glm::vec3 s, glm::vec3 e, float maxOffset, int depth) {
    if (depth == 0)
        return;
    
    vec3 mid = (s + e) / 2.0f + vec3(rand(-maxOffset, maxOffset),
                                     rand(-maxOffset, maxOffset),
                                     rand(-maxOffset, maxOffset));
    
    GenKeyPoints(s, mid, maxOffset / 2, depth - 1);
    particles.push_back(Particle(mid, vec3(0), vec3(0), 1.0));
    GenKeyPoints(mid, e, maxOffset / 2, depth - 1);
}

void Bolt::Update(float elapsedTime) {
    color -= vec3(elapsedTime);
}

bool Bolt::Valid() {
    return color.x > 0.0f;
}

void Bolt::Draw(const Program& p, const glm::mat4& viewProjection,
                const glm::vec3& cameraPos, bool glowMap) {
    lock_guard<std::mutex> lock(mutex);
    
    // Draw lightning bolt
    if (particles.size() == 0)
        return;
    
    if (!model) {
        // pre allocate space for vectors
        vector<vec3> vertices(particles.size());
        vector<size_t> indices(particles.size());
        
        // add vertex for each key point
        for (size_t i = 0; i < particles.size(); i++) {
            Particle keyPoint = particles[i];
            
            vertices[i] = keyPoint.location;
            indices[i] = i;
        }
        
        ArrayBuffer<vec3> ab(vertices);
        ElementArrayBuffer eab(indices);
        model = new Model(ModelBuffer(ab, eab), Material(), Bounds());
    }
    
    mat4 MVP = viewProjection;
    
    p.SetMVP(MVP);
    
    p.SetUniform("baseColor", color);
    p.SetUniform("illum", 0);
    glLineWidth(3.0);
    model->Draw(p, GL_LINE_STRIP);
}

bool BulletCluster::Intersects(Object *object)
{
    lock_guard<std::mutex> lock(mutex);

	if (particles.empty()) return false;
    
    // Check if bullet is in object's bounding box
    Bounds bounds = object->GetBounds();
    
    for (int i = 0; i < particles.size(); i++) {
        Particle& bullet = particles[i];
        
        // f3 and b1 correspond to the max and min
        // corners of the bounding boxes, respectively
        if (bullet.location.x < bounds.b1.x ||
            bounds.f3.x < bullet.location.x)
            continue;
        if (bullet.location.y < bounds.b1.y ||
            bounds.f3.y < bullet.location.y)
            continue;
        if (bullet.location.z < bounds.b1.z ||
            bounds.f3.z < bullet.location.z)
            continue;
        return true;
    }
    
    return false;
}

void BulletCluster::AddBullet(glm::vec3 l, glm::vec3 v)
{
    lock_guard<std::mutex> lock(mutex);
    
    particles.push_back(Particle(l, v, vec3(0), 1.0));
}

void BulletCluster::Draw(const Program& p, const glm::mat4& viewProjection,
                  const glm::vec3& cameraPos, bool glowMap)
{
    lock_guard<std::mutex> lock(mutex);
    
    // Draw bullets
    if (particles.size() == 0)
        return;
    
    if (!model) {
        // pre allocate space for vectors
        vector<vec3> vertices(particles.size() * 2);
        vector<size_t> indices(particles.size() * 2);
        
        // add triangle for each particle
        for (size_t i = 0; i < particles.size(); i++) {
            Particle bullet = particles[i];
            
            vertices[i * 2] = bullet.location;
            vertices[i * 2 + 1] = bullet.location - normalize(bullet.velocity);
            
            indices[i * 2] = i * 2;
            indices[i * 2 + 1] = i * 2 + 1;
        }
        
        ArrayBuffer<vec3> ab(vertices);
        ElementArrayBuffer eab(indices);
        model = new Model(ModelBuffer(ab, eab), Material(), Bounds());
    }
    
    mat4 MVP = viewProjection;
    
    p.SetMVP(MVP);
    
    p.SetUniform("baseColor", color);
    p.SetUniform("illum", 0);
    
    if (glowMap)
        glLineWidth(10.0);
    else
        glLineWidth(3.0);
    model->Draw(p, GL_LINES);
}

void ParticleSystem::Update(float elapsedTime)
{
	for (int i = 0; i < clusters.size(); i++ ) {
		ParticleCluster *cluster = clusters[i];
		if (!cluster->Valid()) {
            delete cluster;
			clusters.erase(clusters.begin() + i--);
        }
		else
			cluster->Update(elapsedTime - lastTime);
	}
    
    lastTime = elapsedTime;
}

void ParticleSystem::AddBulletCluster(BulletCluster *cluster)
{
    clusters.push_back(cluster);
}

void ParticleSystem::AddExplosionCluster(glm::vec3 location, glm::vec3 color)
{
    ParticleCluster *cluster = new ParticleCluster(location, color);
    clusters.push_back(cluster);
}

void ParticleSystem::AddBolt(glm::vec3 start, glm::vec3 end)
{
    clusters.push_back(new Bolt(start, end));
}

bool ParticleSystem::Intersects(Object *object)
{
    for (std::vector<ParticleCluster *>::iterator it = clusters.begin();
         it != clusters.end();
         it++)
    {
        ParticleCluster *cluster = *it;
        if (cluster->Intersects(object))
            return true;
    }
    return false;
}

void ParticleSystem::Draw(const Program& p, const glm::mat4& viewProjection,
          const glm::vec3& cameraPos, bool glowMap)
{
    for (std::vector<ParticleCluster *>::iterator it = clusters.begin();
         it != clusters.end();
         it++)
    {
        ParticleCluster *cluster = *it;
        cluster->Draw(p, viewProjection, cameraPos, glowMap);
    }
}