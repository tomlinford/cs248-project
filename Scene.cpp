#include "Scene.h"

using namespace::glm;

Scene::Scene(Player p)
{
    player = p;
}

Scene::~Scene()
{
}

void Scene::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Scene::SetView(mat4 v)
{
    view = v;
}

void Scene::SetProjection(mat4 p)
{
    projection = p;
}
