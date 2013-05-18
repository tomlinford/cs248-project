#include "Scene.h"

using namespace::glm;

Scene::Scene(Player p)
{
    player = p;
    main = new Program("Shaders/phong.vert", "Shaders/phong.frag");
    
    SetView(lookAt(vec3(0, 1, 2),   // Eye
                   vec3(0, 0, 0),   // Apple
                   vec3(0, 1, 0))); // Up
}

Scene::~Scene()
{
    delete main;
}

void Scene::LoadLevel(Level *l)
{
    level = l;
}

void Scene::Update()
{
}

void Scene::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    main->Use();
    main->SetUniform("illum", 1);
    main->SetUniform("baseColor", vec3(0, 0.9, 0));
    main->SetUniform("lightPosition", vec3(0, 3, 0));
    main->SetUniform("cameraPosition", vec3(0, 1, 2));
    
    // Not currently needed
    // main->SetView(view);
    // main->SetProjection(projection);
    
    // Some levels may be in space and not have a terrain map
    if (level->map)
        level->map->Draw(*main, projection * view, vec3(0, 0, 5));
    
    // Draw ship
    if (level->ship)
        level->ship->Draw(*main, projection * view, vec3(0, 0, 5));
    
    // Draw objects in scene
    for (std::vector<Object>::iterator it = level->objects.begin();
         it != level->objects.end();
         it++)
    {
        Object obj = *it;
        obj.Draw(*main, projection * view, vec3(0, 0, 5));
    }
    
    main->Unuse();
}

void Scene::SetView(mat4 v)
{
    view = v;
}

void Scene::SetProjection(mat4 p)
{
    projection = p;
}
