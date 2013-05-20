#include "Scene.h"
#include <boost/timer.hpp>

using namespace::glm;

Scene::Scene(Player p) : terrain(NULL)
{
    player = p;
    main = new Program("Shaders/main.vert", "Shaders/main.frag");
    
    SetView(lookAt(vec3(.7, .7, .7),   // Eye
                   vec3(0, 0, 0),   // Apple
                   vec3(0, 0, 1))); // Up
}

Scene::~Scene()
{
    if (main)
        delete main;
    if (level)
        delete level;
}

void Scene::LoadLevel(Level *l)
{
    level = l;
}

// TODO: Animation & player-specific camera updates
void Scene::Update()
{
}

void Scene::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (terrain)
		terrain->Draw(projection * view);
    
    main->Use();
    main->SetUniform("illum", 1);
    main->SetUniform("baseColor", vec3(0, 0.9, 0));
    main->SetUniform("lightPosition", vec3(0, 3, 0));
    main->SetUniform("cameraPosition", vec3(0, 1, 2));
    
    // Not currently needed
    // main->SetView(view);
    // main->SetProjection(projection);
    
    // Some levels may be in space and not have a terrain map
	level->DrawMap(projection * view, vec3(0.f, 0.f, 5.f));
    
    // Draw ship
    //if (level->ship)
        //level->ship->Draw(*main, projection * view, vec3(0, 0, 5));
    
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

void Scene::SetTerrain(float *terrainMap, size_t size) {
	if (terrain != NULL) delete terrain;
	terrain = new Terrain(terrainMap, size);
}
