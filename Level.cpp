#include "Level.h"

using namespace glm;

void Level::SetLevel(float *terrainMap, size_t size) {
	boost::mutex::scoped_lock scoped_lock(mapLoader.mutex);
	mapLoader.needsToLoad = true;
	mapLoader.terrainMap = terrainMap;
	mapLoader.size = size;
}

void Level::DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos) {
	if (map) {
		map->Draw(viewProjection, cameraPos);
		return;
	}
	boost::mutex::scoped_lock scoped_lock(mapLoader.mutex);
	if (!mapLoader.needsToLoad) return;
	map = new Map(mapLoader.terrainMap, mapLoader.size);
	mapLoader.needsToLoad = false;
	map->Draw(viewProjection, cameraPos);
	delete[] mapLoader.terrainMap;
}