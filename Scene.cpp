#include "Scene.h"

#define MAX_X 2.4
#define MAX_Y 1.8

using namespace::std;
using namespace::glm;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;

static int count;

Scene::Scene(Player p) : particle_sys()
{
    player = p;
    theta = phi = 0.0f;
    frustum = new Frustum();
    main = new Program("Shaders/main.vert", "Shaders/main.frag");
}

Scene::~Scene()
{
    if (main)
        delete main;
    if (level)
        delete level;
    if (frustum)
        delete frustum;
}

void Scene::LoadLevel(Level *l)
{
    level = l;
	level->Load();
}

/* Scene update functions below */

/** Captures ship offset based on key presses. */
void Scene::HandleKeys()
{
    if (keyLeft)
        shipOffset.x -= 0.1;
    if (keyRight)
        shipOffset.x += 0.1;
    if (keyUp)
        shipOffset.y += 0.1;
    if (keyDown)
        shipOffset.y -= 0.1;
    
    shipOffset = clamp(shipOffset, vec2(-MAX_X, -MAX_Y), vec2(MAX_X, MAX_Y));
}

/** Updates objects by moving them to their new locations,
 which is a function of the elapsed time. */
void Scene::UpdateObjects(float elapsedSeconds)
{
    // Compute ship position along path
    vec3 direction = level->GetDirection(level->ship, elapsedSeconds);
    vec3 position = level->GetPosition(level->ship, elapsedSeconds);
    
    // Update light/camera
    lightPosition = position;
    cameraPosition = position - 3.0f * direction;
    
    // Update ship position
    // Important: Note offset and direction must be set before
    // position
    level->ship->SetOffset(shipOffset);
    level->ship->SetDirection(direction);
    level->ship->SetPosition(position);
    
    // Update other objects
    for (std::vector<Object>::iterator it = level->objects.begin();
         it != level->objects.end();
         it++)
    {
        Object obj = *it;
        bool inFrustrum = length(cameraPosition - obj.GetPosition()) > 50;
        obj.SetInFrustrum(inFrustrum);
        
        // TODO
    }
    
    // Update particles
    particle_sys.Update(elapsedSeconds);
}

/** Determine if a collision has occurred - if so,
 determine which objects to remove from the scene, and
 whether to add particle effects at the location of
 collision. */
void Scene::HandleCollisions()
{
}

/** Updates the player views, which depends on the
 ship position. */
void Scene::UpdateView(float elapsedSeconds)
{
    // View for player 1 (chase cam)
    if (player == PLAYER1)
    {
        // Compute ship position along path
        vec3 position = level->GetPosition(level->ship, elapsedSeconds);
        quat orientation = level->ship->GetOrientation();
        vec3 direction = orientation * vec3(0, 0, -1);
        vec3 up = orientation * vec3(0, 1, 0);
    
        SetView(lookAt(position - 3.0f * direction,
                       position + direction,
                       up));
        
        frustum->LoadCam(position - 3.0f * direction,
                         position + direction,
                         up);
    }
    // View for player 2 (on board cam)
    else
    {
        // Compute view vectors
        vec3 position = level->ship->GetPosition();
        quat orientation = level->ship->GetOrientation() * fquat(vec3(phi, theta, 0));
        vec3 direction = orientation * vec3(0, 0, -1);
        vec3 up = orientation * vec3(0, 1, 0);
        
        SetView(lookAt(position + 0.3f * up,
                       position + 0.3f * up + direction,
                       up));
        
        frustum->LoadCam(position - 3.0f * direction,
                         position + direction,
                         up);
    }
}

void Scene::Update()
{
    times = timer.elapsed();
    float elapsedSeconds = (float)times.wall / pow(10.f, 9.f);
    
    HandleKeys();
    UpdateObjects(elapsedSeconds);
    HandleCollisions();
    UpdateView(elapsedSeconds);
}

/* Rendering functions below */

void Scene::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Maps have their own shader program for vertex displacement
	level->DrawMap(projection * view, cameraPosition, lightPosition, *frustum);
    
    mat4 viewProjection = projection * view;
    
    main->Use();
    main->SetUniform("illum", 1);
    main->SetUniform("lightPosition", lightPosition);
    main->SetUniform("cameraPosition", cameraPosition);
    
    // Not currently needed
    // main->SetView(view);
    // main->SetProjection(projection);
    
    // Draw ship
    if (level->ship && frustum->Contains(*level->ship))
    {
        level->ship->Draw(*main, viewProjection, cameraPosition);
        level->ship->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
    }
    
    // Draw objects in scene
    for (std::vector<Object>::iterator it = level->objects.begin();
         it != level->objects.end();
         it++)
    {
        Object obj = *it;
        if (frustum->Contains(obj))
            obj.Draw(*main, viewProjection, cameraPosition);
    }
    
    // Draw particles
    ::count++;
    if (::count % 100 == 0) {
        particle_sys.AddExplosionCluster(level->ship->GetPosition(), vec3(0.0, 0.9, 0.0));
    }
    particle_sys.Draw(*main, viewProjection, cameraPosition);
    
    main->Unuse();
}
