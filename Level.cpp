#include "Level.h"
#include "CMSpline.hpp"

using namespace glm;
using namespace std;

Level::Level()
{
    mapLoader.needsToLoad = false;
    
    for (int i = 0; i < 10 * M_PI + 1; i += M_PI / 3) {
        ControlPoint point;
        point.time = i * 2;
        point.position = glm::vec3(5 * std::cos(i), (float)rand() / RAND_MAX * 2, 5 * std::sin(i));
        path.push_back(point);
        if (i == 0 || i == (int)(10 * M_PI)) {
            path.push_back(point);
        }
    }
}

void Level::SetLevel(float *terrainMap, size_t size) {
	lock_guard<mutex> lock(mapLoader.mutex);
	mapLoader.needsToLoad = true;
	mapLoader.terrainMap = terrainMap;
	mapLoader.size = size;
}

vec3 Level::GetPosition(Flyable *flyable, float time)
{
    int i = 0;
    while (i < path.size() && path[i].time < time)
        i++;
    
    // Exclude edge cases where we don't have enough
    // control points
    if (i - 2 >= 0 &&
        i + 1 < path.size())
    {
        float u = (time - path[i - 1].time) /
                  (path[i].time - path[i - 1].time);
        
        CMSpline spline(path[i - 2].position,
                        path[i - 1].position,
                        path[i].position,
                        path[i + 1].position);
        
        return spline.evaluate3D(u);
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
        i + 1 < path.size())
    {
        float u = (time - path[i - 1].time) /
        (path[i].time - path[i - 1].time);
        
        CMSpline spline(path[i - 2].position,
                        path[i - 1].position,
                        path[i].position,
                        path[i + 1].position);
        
        return spline.tangent3D(u);
    }
    
    // Set to no direction if we fail
    return vec3(0.0);
}

void Level::DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos) {
	if (map) {
		map->Draw(viewProjection, cameraPos);
		return;
	}
	lock_guard<mutex> lock(mapLoader.mutex);
	if (!mapLoader.needsToLoad) return;
	map = new Map(mapLoader.terrainMap, mapLoader.size);
	mapLoader.needsToLoad = false;
	map->Draw(viewProjection, cameraPos);
	delete[] mapLoader.terrainMap;
}