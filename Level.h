#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <thread>
#include <mutex>

#include "CMSpline.hpp"
#include "Object.h"
#include "Flyable.h"
#include "Map.h"

/** Defines a control point along a route */
struct ControlPoint
{
public:
    glm::vec3 position;     // In world space
    glm::quat orientation;  // Multiplier against previous orientation
    CMSpline *spline;       // Spline to use for this control point (will need to be freed)
    float time;             // In elapsed seconds since start
};

/** Loads map information; used in conjunction with
 Networking class */
struct MapLoader
{
    bool needsToLoad;       // Has the map been loaded yet?
    float *terrainMap;      // Will need to be freed
    size_t size;            // size of the map (32, 64, 128...)
    std::mutex mutex;
	int x, y; // x and y positions
};

/** Defines a game level */
class Level
{
public:
	Level();
    ~Level();

    /** Gets a flyable object's position as a function of time */
    glm::vec3 GetPosition(Flyable *flyable, float time);
    
    /** Gets a flyable object's direction as a function of time 
     (Assuming orientation is aligned along path axis */
    glm::vec3 GetDirection(Flyable *flyable, float time);

	/* This will be called from another thread, so Level will have to hold onto this */
	void SetLevel(float *terrainMap, size_t size, int x, int y);

	/** This will be called from the main thread, so the Level will load the map if necessary */
	void DrawMap(const glm::mat4& viewProjection, const glm::vec3& cameraPos);

    std::vector<Map *> maps;
    Ship *ship;
    std::vector<Object> objects;
    std::vector<ControlPoint> path;

private:
	MapLoader mapLoader;
    
    /** Precomputes splines. Call after loading control points. */
    void PrecomputeSplines();
};