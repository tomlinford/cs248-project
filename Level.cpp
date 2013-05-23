#include "Level.h"

using namespace glm;
using namespace std;

void Level::SetLevel(float *terrainMap, size_t size, int x, int y) {
	lock_guard<mutex> lock(mapLoader.mutex);
	mapLoader.needsToLoad = true;
	mapLoader.terrainMap = terrainMap;
	mapLoader.size = size;
	mapLoader.x = x;
	mapLoader.y = y;
}

void Level::DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos) {
	lock_guard<mutex> lock(mapLoader.mutex);
	if (maps.size() > 0 && !mapLoader.needsToLoad) {
		for (Map *map : maps) map->Draw(viewProjection, cameraPos);
		return;
	}
	if (!mapLoader.needsToLoad) return;
	maps.push_back(new Map(mapLoader.terrainMap, mapLoader.size, mapLoader.x, mapLoader.y));
	mapLoader.needsToLoad = false;
	for (Map *map : maps) map->Draw(viewProjection, cameraPos);
	delete[] mapLoader.terrainMap;
}