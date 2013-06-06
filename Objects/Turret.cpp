#include "Turret.h"

Turret::Turret(Model *model) : Object(model)
{
    cluster = new BulletCluster();
}

Turret::Turret(const string& filename) : Object(filename)
{
    cluster = new BulletCluster();
}

void Turret::SetColor(glm::vec3 c) {
    Object::SetColor(c);
    cluster->SetColor(c);
}

void Turret::AddBullet(glm::vec3 location, glm::vec3 velocity)
{
    cluster->AddBullet(location, velocity);
}