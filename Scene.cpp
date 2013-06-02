#include "Scene.h"

#include <sstream>
#include <thread>
#include <boost/asio.hpp>

#include "Networking.h"

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
    level = NULL;
    timer = NULL;
    
    // Spawn update thread
	thread updateThread(&Scene::Update, this);
	updateThread.detach();
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
    cond.notify_all();
    particle_sys.AddBulletCluster(level->ship->GetBulletCluster());
}

/* Scene update functions below */

/** Captures ship offset based on key presses. */
void Scene::HandleKeys(float elapsedSeconds)
{
    float interval = 3 * (elapsedSeconds - lastTime);
    
    if (keyLeft)
        shipOffset.x -= interval;
    if (keyRight)
        shipOffset.x += interval;
    if (keyUp)
        shipOffset.y += interval;
    if (keyDown)
        shipOffset.y -= interval;
    
    shipOffset = glm::clamp(shipOffset, vec2(-MAX_X, -MAX_Y), vec2(MAX_X, MAX_Y));
}

/** Adds bullets based on mouse presses. */
void Scene::HandleMouse(float elapsedSeconds)
{
    float interval = 3 * (elapsedSeconds - lastTime);
    
    if (mouseLeft && level->ship) {
        vec3 selected = glm::unProject(vec3(512, 384, 1.0),
                                       view,
                                       projection,
                                       vec4(0, 0, 1024 ,768));
        vec3 velocity = 20.0f * normalize(selected - level->ship->GetPosition());
        level->ship->AddBullet(level->ship->GetPosition(), velocity);
		Networking::AddBullet(level->ship->GetPosition(), velocity);
    }
    if (mouseRight && level->ship) {
        
    }
}

/** Updates objects by moving them to their new locations,
 which is a function of the elapsed time. */
void Scene::UpdateObjects(float elapsedSeconds)
{
    // Compute ship position along path
    if (level->ship)
    {
        vec3 shipDirection = level->GetDirection(FORWARD, elapsedSeconds);
        vec3 shipPosition = level->GetPosition(FORWARD, elapsedSeconds);
        
        // Update light/camera
        lightPosition = shipPosition;
        cameraPosition = shipPosition - 3.0f * shipDirection;
        
        // Update ship position
        // Important: Note offset and direction must be set before
        // position
        level->ship->SetOffset(shipOffset);
        level->ship->SetDirection(shipDirection);
        level->ship->SetPosition(shipPosition);
    }
    
    // Update other objects
    for (std::vector<Object *>::iterator it = level->objects.begin();
         it != level->objects.end();
         it++)
    {
        Object *obj = *it;
        Flyable *flyable = dynamic_cast<Flyable *>(obj);
        Missile *missile = dynamic_cast<Missile *>(obj);
        
        if (missile && level->ship)
        {
            vec2 missileOffset = missile->GetOffset();
            vec2 shipOffset = level->ship->GetOffset();
            shipOffset.x *= -1;
            
            vec2 dir = shipOffset - missileOffset;
            vec2 offset = 0.01f * dir;
            missile->SetOffset(missileOffset + offset);
        }
        if (flyable)
        {
            vec3 direction = level->GetDirection(BACKWARD, elapsedSeconds + flyable->GetTimeOffset());
            vec3 position = level->GetPosition(BACKWARD, elapsedSeconds + flyable->GetTimeOffset());
            
            // Remove objects off screen
            if (position.x - 0.0 < 0.0001 &&
                position.y - 0.0 < 0.0001 &&
                position.z - 0.0 < 0.0001)
            {
                it = level->objects.erase(it);
                it--;
            }
            else
            {
                flyable->SetDirection(direction);
                flyable->SetPosition(position);
            }
        }
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
    // Check map collision
    for (int i = 0; i < level->maps.size(); i++)
    {
        Map *map = level->maps[i];
        
        // If the map is not in the frustum,
        // skip it for now
        if (!frustum->Contains(*map))
            continue;
        
        // Check ship intersections
        if (level->ship && map->Intersects(*level->ship))
            particle_sys.AddExplosionCluster(level->ship->GetPosition(), map->GetColor());
        
        // Check object collisions
        for (int i = 0; i < level->objects.size(); i++)
        {
            Object *obj = level->objects[i];
            Missile *missile = dynamic_cast<Missile *>(obj);
            
            // If the object is not in the frustum,
            // skip it for now
            if (!frustum->Contains(*obj))
                continue;
            
            // Check object-bullet intersections
            if (particle_sys.Intersects(obj)) {
                particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
                delete obj;
                obj = NULL;
                level->objects.erase(level->objects.begin() + i--);
                continue;
            }
            
            // Check object-ship intersections
            if (level->ship && level->ship->Intersects(*obj))
            {
                if (missile)
                {
                    particle_sys.AddExplosionCluster(level->ship->GetPosition(), level->ship->GetColor());
                    //delete level->ship;
                    //level->ship = NULL;
                }
                particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
                delete obj;
                obj = NULL;
                level->objects.erase(level->objects.begin() + i--);
                continue;
            }
            
            // Check object-map intersections
            if (map->Intersects(*obj)) {
                //particle_sys.AddExplosionCluster(obj->GetPosition(), map->GetColor());
                //delete obj;
                //level->objects.erase(level->objects.begin() + i--);
            }
        }
    }
}

/** Updates the player views, which depends on the
 ship position. */
void Scene::UpdateView(float elapsedSeconds)
{
    // View for player 1 (chase cam)
    if (player == PLAYER1 && level->ship)
    {
        // Compute ship position along path
        vec3 position = level->GetPosition(FORWARD, elapsedSeconds);
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
    else if (level->ship)
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

/* Updates object positions in world based on elapsed
 time and performs collision testing on a separate
 thread */
void Scene::Update()
{
    while (true) {
        unique_lock<std::mutex> lock(mutex);
        
        // Wait until the level is ready
        while (!level || !level->Ready())
        {
            cond.wait(lock);
			if (timer)
                delete timer;
			timer = new cpu_timer();
        }
		
        times = timer->elapsed();
		float elapsedSeconds = (float)times.wall / pow(10.f, 9.f);
        
        // Handle player-specific input
        if (player == PLAYER1)
            HandleKeys(elapsedSeconds);
        else
            HandleMouse(elapsedSeconds);
        
        UpdateObjects(elapsedSeconds);
        HandleCollisions();
        UpdateView(elapsedSeconds);
        
        lastTime = elapsedSeconds;
    }
}

/* Loads new objects via the main thread because OpenGL
 calls are not thread safe */
void Scene::LoadNewObjects()
{
    float size = level->objects.size();
    for (int i = 0; i < size; i++)
    {
        Ship *ship = dynamic_cast<Ship *>(level->objects[i]);
        if (ship &&
            level->ship &&
            ::count % 30 == 0 &&
            glm::distance(level->ship->GetPosition(), ship->GetPosition()) < 20)
        {
            Missile *missile = new Missile("Models/missile.obj");
            missile->SetColor(vec3(1.0, 1.0, 1.0));
            missile->SetTimeOffset(ship->GetTimeOffset());
            missile->SetOffset(ship->GetOffset());
            level->objects.push_back(missile);
        }
    }
}

/* Rendering functions below */

void Scene::Render()
{
	if (!timer) return;
    unique_lock<std::mutex> lock(mutex);
    
    LoadNewObjects();
    
    ::count++;
    
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
    
    glLineWidth(2.0f);
    
    // Draw ship
    if (level->ship && frustum->Contains(*level->ship))
    {
        level->ship->Draw(*main, viewProjection, cameraPosition);
        level->ship->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
    }
    
    std::vector<Flyable *> missiles;
    
    // Draw objects in scene
    for (std::vector<Object *>::iterator it = level->objects.begin();
         it != level->objects.end();
         it++)
    {
        Object *obj = *it;
        if (frustum->Contains(*obj))
        {
            main->SetUniform("illum", 1);
            obj->Draw(*main, viewProjection, cameraPosition);
            obj->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
        }
    }
    
    for (int i = 0; i < missiles.size(); i++) {
        level->objects.push_back(missiles[i]);
    }
    
    // Draw particles
    particle_sys.Draw(*main, viewProjection, cameraPosition);
    
    main->Unuse();
}
