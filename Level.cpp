#include "Level.h"
#include "CMSpline.hpp"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

using namespace glm;
using namespace std;

Level::Level() : ready(false)
{
}
    
Level::~Level()
{
    for (int i = 2; i < path.size() - 1; i++) {
        delete path[i].spline;
        }
    }
    
void Level::Load() {
	unique_lock<std::mutex> lock(mutex);
	while(!ready) cond.wait(lock);
}

void Level::SetControlPoints(const glm::vec3 *points, size_t num) {
	for (size_t i = 0; i < num; i += 4) {
		ControlPoint point;
		point.time = float(i);
		point.position = vec3(points[i].x * 20, (points[i].z - .5) * 20 + 10, points[i].y * 20);
		path.push_back(point);
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

vec3 Level::GetPosition(Flyable *flyable, float time)
{
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
    return vec3(0.0);
}

glm::vec3 Level::GetDirection(Flyable *flyable, float time)
{
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
        return path[i].spline->tangent3D(u);
    }
    
    // Set to no direction if we fail
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
}

void Level::DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos,
                    const glm::vec3& lightPos) {
	lock_guard<std::mutex> lock(mutex);
	if (maps.size() > 0 && mapLoaders.size() == 0) {
		for (Map *map : maps) map->Draw(viewProjection, cameraPos, lightPos);
		return;
	}
	if (mapLoaders.size() == 0) return;
	for (MapLoader &mapLoader : mapLoaders) {
	Map *newMap = new Map(mapLoader.terrainMap, mapLoader.size, mapLoader.x, mapLoader.y);
	newMap->SetColor(vec3(0.0, 0.4, 0.5));
	maps.push_back(newMap);
	mapLoader.needsToLoad = false;
		delete[] mapLoader.terrainMap;
	}
	mapLoaders.clear();
	for (Map *map : maps) map->Draw(viewProjection, cameraPos, lightPos);
	cond.notify_all();
}

void Level::SetReady() {
	ready = true;
	cond.notify_all();
}