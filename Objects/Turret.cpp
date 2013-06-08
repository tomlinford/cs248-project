#include "Turret.h"

Turret::Turret(Model *model) : Object(model)
{
    health = 20.0;
    cluster = new BulletCluster();
    cluster->SetOwner(this);
}

Turret::Turret(const string& filename) : Object(filename)
{
    health = 20.0;
    cluster = new BulletCluster();
    cluster->SetOwner(this);
}

void Turret::SetColor(glm::vec3 c) {
    Object::SetColor(c);
    cluster->SetColor(c);
}

void Turret::AddBullet(glm::vec3 location, glm::vec3 velocity)
{
    cluster->AddBullet(location, velocity);
}