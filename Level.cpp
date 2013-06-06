#include "Level.h"
#include "CMSpline.hpp"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

using namespace glm;
using namespace std;

float rand2(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

Level::Level() : ready(false)
{
    ship = new Ship("Models/ship.obj");
    ship->SetColor(vec3(0.0, 0.9, 0.0));
    
    sphere = new Object("Models/icosphere.obj");
    sphere->SetColor(vec3(1.0, 0.0, 1.0));
    sphere->SetPosition(vec3(150, 0, 150));
    sphere->SetScale(75.0f);
    
    // Random turrets
    // (Take this out)
    for (int i = 0; i < 20; i++)
    {
        Turret *turret = new Turret("Models/turret.obj");
        turret->SetColor(vec3(0.9, 0.7, 0.5));
        turret->SetPosition(vec3(rand2(0, 300),
                                 0,
                                 rand2(0, 300)));
        turret->SetScale(5.0);
        objects.push_back(turret);
    }
}

Level::~Level()
{
    for (int i = 2; i < path.size() - 1; i++) {
        delete path[i].spline;
    }
}
    
void Level::Load() {
	unique_lock<std::mutex> lock(mutex);
	while(!ready) {
        cond.wait(lock);
		LoadMaps();
	}
}

void Level::SetControlPoints(const glm::vec3 *points, size_t num) {
	for (size_t i = 0; i < num; i += 4) {
		ControlPoint point;
		point.time = float(i) * 0.5;
		point.position = vec3(points[i].x * 20, (points[i].z - .5) * 60 + 4, points[i].y * 20);
		path.push_back(point);
        
        // Duplicate start and end
        if (i == 0 || i + 4 >= num) {
            path.push_back(point);
            totalTime = float(i) * 0.5;
        }
    }
	PrecomputeSplines();
}

void Level::PrecomputeSplines()
{
    // Precompute splines
    for (int i = 2; i < path.size() - 1; i++) {
        path[i].spline = new CMSpline(path[i - 2].position,
                                      path[i - 1].position,
                                      path[i].position,
                                      path[i + 1].position);
    }
}

vec3 Level::GetPosition(Direction direction, float time)
{
    if (direction == BACKWARD)
        time = totalTime - time;
    
    if (time > totalTime)
        time = totalTime - 10;
    
    int i = 0;
    while (i < path.size() && path[i].time < time)
        i++;
    
    // Exclude edge cases where we don't have enough
    // control points
    if (i - 2 >= 0 &&
        i + 1 < path.size() &&
        path[i].spline)
    {
        float u = (time - path[i - 1].time) /
                  (path[i].time - path[i - 1].time);
        return path[i].spline->evaluate3D(u);
    }
    
    // Send to origin if we fail (for debugging)
    cerr << "Could not calculate direction for time " << time << endl;
    return vec3(0.0);
}

glm::vec3 Level::GetDirection(Direction direction, float time)
{
    if (direction == BACKWARD)
        time = totalTime - time;
    
    if (time > totalTime)
        time = totalTime - 10;
    
    int i = 0;
    while (i < path.size() && path[i].time < time)
        i++;
    
    // Exclude edge cases where we don't have enough
    // control points
    if (i - 2 >= 0 &&
        i + 1 < path.size() &&
        path[i].spline)
    {
        float u = (time - path[i - 1].time) /
                  (path[i].time - path[i - 1].time);
        
        vec3 dir = path[i].spline->tangent3D(u);
        if (direction == BACKWARD)
            dir *= -1.0f;
        return dir;
    }
    
    // Set to no direction if we fail
    cerr << "Could not calculate direction for time " << time << endl;
    return vec3(0.0);
}

void Level::SetLevel(float *terrainMap, size_t size, int x, int y) {
	lock_guard<std::mutex> lock(mutex);
	MapLoader mapLoader;
	mapLoader.needsToLoad = true;
	mapLoader.terrainMap = terrainMap;
	mapLoader.size = size;
	mapLoader.x = x;
	mapLoader.y = y;
	mapLoaders.push_back(mapLoader);
    
	cond.notify_all();
}

void Level::AddEnemyShip(float timeOffset, glm::vec2 offset) {
	Ship *ship = new Ship("Models/ship.obj");
    ship->SetColor(vec3(0.9, 0.0, 0.0));
    ship->SetTimeOffset(timeOffset);
    ship->SetOffset(offset);
    objects.push_back(ship);
}

void Level::DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos,
                    const glm::vec3& lightPos, const Frustum& frustum, bool glowMap) {
	int count = 0;
    for (Map *map : maps) {
        if (frustum.Contains(*map))
            map->Draw(viewProjection, cameraPos, lightPos, glowMap);
        else
            count++;
            
    }
    //cout << "Culled " << count << " sub maps " << endl;
}

void Level::LoadMaps() {
	for (MapLoader &mapLoader : mapLoaders) {
        Map *newMap = new Map(mapLoader.terrainMap, mapLoader.size, mapLoader.x, mapLoader.y);
        newMap->SetColor(vec3(0.0, 0.20, 0.25));
        
        if (sphere)
            newMap->SetControlFieldPosition(sphere->GetPosition());
        
        maps.push_back(newMap);
        mapLoader.needsToLoad = false;
        
        // We need this data for collision testing - don't delete!
        //delete[] mapLoader.terrainMap;
	}
	mapLoaders.clear();
}

void Level::SetReady() {
	ready = true;
	cond.notify_all();
}