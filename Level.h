#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Object.h"
#include "Flyable.h"
#include "Map.h"

class Level
{
public:
    Map *map;
    Ship *ship;
    std::vector<Object> objects;
    std::vector<glm::vec3> path;
};