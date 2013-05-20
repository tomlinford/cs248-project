#include "Scene.h"

using namespace::glm;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;

Scene::Scene(Player p) 
{
    player = p;
    main = new Program("Shaders/main.vert", "Shaders/main.frag");
    
    SetView(lookAt(vec3(-3, 2, -3),   // Eye
                   vec3(0, 0, 0),   // Apple
                   vec3(0, 1, 0))); // Up
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
    times = timer.elapsed();
    float elapsedSeconds = (float)times.wall / pow(10, 9);
    
    // Ship position
    vec3 position = level->GetPosition(level->ship, elapsedSeconds);
    vec3 direction = level->GetDirection(level->ship, elapsedSeconds);
    level->ship->SetPosition(position);
    level->ship->SetDirection(direction);
    
    // Set view for player 1 (chase cam)
    if (player == PLAYER1) {
        vec3 up = vec3(0, 1, 0);
        SetView(lookAt(position + up - 2.0f * direction,
                       position + up + direction,
                       up));
    }
    else {
        
    }
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
	level->DrawMap(projection * view, vec3(0.f, 0.f, 5.f));
    
    // Draw ship
    if (level->ship) {
        level->ship->Draw(*main, projection * view, vec3(0, 0, 5));
    }
    
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
