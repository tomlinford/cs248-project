#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "CMSpline.hpp"
#include "Object.h"
#include "Flyable.h"
#include "Map.h"
#include "Turret.h"

#include "Utilities/Frustum.h"

/* Defines a direction of travel along the path */
enum Direction {
    FORWARD,
    BACKWARD
};

/** Defines a control point along a route */
struct ControlPoint
{
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
	int x, y;               // x and y positions
};

/** Defines a game level */
class Level
{
public:
	Level(bool addSphere);
    ~Level();

    /** Gets a flyable object's position as a function of time */
    glm::vec3 GetPosition(Direction direction, float time);
    
    /** Gets a flyable object's direction as a function of time 
     (Assuming orientation is aligned along path axis */
    glm::vec3 GetDirection(Direction direction, float time);
    
    /* Updates object positions based on elapsed time */
    void Update(float elapsedSeconds);
    
    /* Checks for overlap between objects and removes those
     that have collided. Effects are added to the provided
     particle system. */
    void HandleCollisions(float elapsedSeconds, ParticleSystem& ps, Frustum& f);

	/* Level loading functions (called from Networking) */
	void Load(); // Will block until level has loaded. Call from main thread
	void LoadMap(float *terrainMap, size_t size, int x, int y);
    void LoadControlPoints(const glm::vec3 *points, size_t num);
    void LoadEnemyShip(float timeOffset, glm::vec2 offset);
    
    /** Whether or not the level has finished loading */
	void SetReady();
    bool Ready();

	/** Height at location between 0, 0 and 1, 1.
		Returns something between 0 and 1*/
	float GetHeightAt(float x, float y);
    
    /** Returns a minimap texture of the given size */
    Texture *GetMinimap(GLuint size);
    
    /** Player score on this level */
    int score;

    /** Level path information */
    float totalTime;
    Model *pathModel;
    std::vector<ControlPoint> path;
    
    /** Level objects */
    Ship *ship;
    Object *sphere;
    std::vector<Map *> maps;
    std::vector<Object *> objects;

	/** Synchronization */
	std::mutex *sceneMutex;
	std::mutex mutex;
	std::condition_variable cond;

private:
	bool ready;
	std::vector<MapLoader> mapLoaders;
    
    /** Private collision testing helpers */
    void CheckMapCollision(float elapsedSeconds, ParticleSystem& ps, Frustum &f);
    void CheckObjectParticleCollisions(ParticleSystem& ps, Frustum &f);
    void CheckObjectShipCollisions(ParticleSystem& ps, Frustum &f);
    
    /** Private level update helpers */
    void UpdateShip(float elapsedSeconds);
    void UpdateSphere(float elapsedSeconds);
    void UpdateObjects(float elapsedSeconds);
    
    /** Private level loading helpers */
    void GenMaps();
    void GenPath();
    void PrecomputeSplines();
};