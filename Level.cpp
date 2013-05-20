#include "Level.h"

using namespace glm;
using namespace std;

void Level::SetLevel(float *terrainMap, size_t size) {
	lock_guard<mutex> lock(mapLoader.mutex);
	mapLoader.needsToLoad = true;
	mapLoader.terrainMap = terrainMap;
	mapLoader.size = size;
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