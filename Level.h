#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>

#include "Object.h"
#include "Flyable.h"
#include "Map.h"

class Level
{
public:
	Level() { mapLoader.needsToLoad = false; }
    std::vector<Map *> maps;
    Ship *ship;
    std::vector<Object> objects;
    std::vector<glm::vec3> path;

	/* This will be called from another thread, so Level will have to hold onto this */
	void SetLevel(float *terrainMap, size_t size, int x, int y);

	/* This will be called from the main thread, so the Level will load the map if necessary */
	void DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos);

private:
	struct {
		bool needsToLoad;
		float *terrainMap; // will need to be freed
		size_t size;
		int x, y;
		std::mutex mutex;
	} mapLoader;
};