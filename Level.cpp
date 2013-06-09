#include "Level.h"
#include "CMSpline.hpp"
#include "Networking.h"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#define MAP_DIM 16
#define GRANULARITY 10

using namespace glm;
using namespace std;

float rand2(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

Level::Level() : ready(false), maps(MAP_DIM * MAP_DIM, NULL)
{
    pathModel = NULL;
    score = 0;
    
    ship = new Ship("Models/ship.obj");
    ship->SetColor(vec3(0.0, 0.9, 0.0));
    
    sphere = new Object("Models/icosphere.obj");
    sphere->SetColor(vec3(1.0, 0.0, 1.0));
    sphere->SetPosition(vec3(150, 0, 150));
    sphere->SetScale(75.0f);

	// Prefetch turret obj file and associated buffer
	Object turret("Models/turret.obj");
}

Level::~Level()
{
    for (int i = 2; i < path.size() - 1; i++) {
        delete path[i].spline;
    }
	delete ship;
	ship = NULL;
	for (Map *&m : maps) {
		Map *temp = m;
		m = NULL;
		delete temp;
	}
	for (Object *&o : objects) {
		Object *temp = o;
		o = NULL;
		delete temp;
	}
	delete sphere;
	sphere = NULL;
}

/************** LEVEL LOADING METHODS HERE **************/

void Level::Load() {
	unique_lock<std::mutex> lock(mutex);
	while(!ready) {
        cond.wait(lock);
		GenMaps();
	}
}

void Level::LoadControlPoints(const glm::vec3 *points, size_t num) {
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

void Level::LoadMap(float *terrainMap, size_t size, int x, int y) {
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

void Level::LoadEnemyShip(float timeOffset, glm::vec2 offset) {
	Ship *ship = new Ship("Models/ship.obj");
    ship->SetColor(vec3(0.9, 0.0, 0.0));
    ship->SetTimeOffset(timeOffset);
    ship->SetOffset(offset);
    objects.push_back(ship);
}

void Level::SetReady()
{
	ready = true;
	cond.notify_all();
}

bool Level::Ready()
{
    return ready;
}

/************** COLLISION TESTING METHODS HERE **************/

void Level::HandleCollisions(float elapsedSeconds, ParticleSystem& ps, Frustum& f)
{
    CheckMapCollision(elapsedSeconds, ps, f);
    CheckObjectParticleCollisions(ps, f);
    CheckObjectShipCollisions(ps, f);
}

void Level::CheckMapCollision(float elapsedSeconds, ParticleSystem& ps, Frustum& f)
{
    if (!ship)
        return;
    
    for (int i = 0; i < maps.size(); i++)
	{
		Map *map = maps[i];
		if (!map || !f.Contains(*map))
			continue;
        
		// Check ship intersections
		if (map->Intersects(*ship)) {
			ps.AddExplosionCluster(ship->GetPosition(), map->GetColor());
            ship->AddDamage(0.001 * elapsedSeconds);
            Networking::SetHealth(ship->GetHealth());
		}
	}
}

void Level::CheckObjectParticleCollisions(ParticleSystem& ps, Frustum& f)
{
    for (int i = 0; i < objects.size(); i++)
	{
		Object *obj = objects[i];
		if (!obj ||
            !f.Contains(*obj) ||
            !ps.Intersects(obj))
			continue;
        
        score += obj->GetValue();
        ps.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
        delete obj;
        obj = NULL;
        objects.erase(objects.begin() + i--);
	}
}

void Level::CheckObjectShipCollisions(ParticleSystem& ps, Frustum& f)
{
    if (!ship)
        return;
    
    for (int i = 0; i < objects.size(); i++)
	{
		Object *obj = objects[i];
		if (!obj ||
            !f.Contains(*obj) ||
            !ship->Intersects(*obj))
			continue;
        
        Ship *enemy = dynamic_cast<Ship *>(obj);
		Missile *missile = dynamic_cast<Missile *>(obj);
        
        if (enemy) ship->AddDamage(0.2);
        else if (missile) ship->AddDamage(0.5);
        
        score += obj->GetValue();
        ps.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
        delete obj;
        obj = NULL;
        objects.erase(objects.begin() + i--);
    }
}

/**************** LEVEL UPDATE METHODS HERE ****************/

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
    // cerr << "Could not calculate direction for time " << time << endl;
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
    // cerr << "Could not calculate direction for time " << time << endl;
    return vec3(0.0);
}


void Level::Update(float elapsedSeconds)
{
    UpdateShip(elapsedSeconds);
    UpdateSphere(elapsedSeconds);
    UpdateObjects(elapsedSeconds);
}

void Level::UpdateShip(float elapsedSeconds)
{
    if (!ship)
        return;
    
    vec3 shipDirection = GetDirection(FORWARD, elapsedSeconds);
    vec3 shipPosition = GetPosition(FORWARD, elapsedSeconds);
    
    ship->SetDirection(shipDirection);
    ship->SetPosition(shipPosition);
}

void Level::UpdateSphere(float elapsedSeconds)
{
    if (!sphere)
        return;
    
    sphere->SetScale(75 + 7 * sin(elapsedSeconds));
}

void Level::UpdateObjects(float elapsedSeconds)
{
    for (int i = 0; i < objects.size(); i++)
	{
		Object *obj = objects[i];
		Missile *missile = dynamic_cast<Missile *>(obj);
		Turret *turret = dynamic_cast<Turret *>(obj);
        Flyable *flyable = dynamic_cast<Flyable *>(obj);
        
		if (missile && ship)
		{
			vec2 missileOffset = missile->GetOffset();
			vec2 shipOffset = ship->GetOffset();
			shipOffset.x *= -1;
            
			vec2 dir = shipOffset - missileOffset;
			vec2 offset = 0.01f * dir;
			missile->SetOffset(missileOffset + offset);
		}
        else if (turret && ship) {
			vec3 turretPos = turret->GetPosition();
			vec3 shipPos = ship->GetPosition();

			if (fabs(glm::distance(shipPos, turretPos)) < 30) {
				vec3 dir = normalize(shipPos - turretPos);
				turret->AddBullet(turretPos, 20.0f * dir);
			}
		}
		if (flyable)
		{
			vec3 direction = GetDirection(BACKWARD, elapsedSeconds + flyable->GetTimeOffset());
			vec3 position = GetPosition(BACKWARD, elapsedSeconds + flyable->GetTimeOffset());
            
			// Remove off-screen objects
			if (position.x - 0.0 < 0.0001 &&
				position.y - 0.0 < 0.0001 &&
				position.z - 0.0 < 0.0001)
			{
				objects.erase(objects.begin() + i--);
			}
			else
			{
				flyable->SetDirection(direction);
				flyable->SetPosition(position);
			}
		}
	}
}

/************ PRIVATE OBJECT GENERATION HELPERS ************/

/* Creates map objects for the streamed terrain data */
void Level::GenMaps() {
	for (MapLoader &mapLoader : mapLoaders) {
        Map *newMap = new Map(mapLoader.terrainMap, mapLoader.size, mapLoader.x, mapLoader.y);
        newMap->SetColor(vec3(0.0, 0.20, 0.25));
        newMap->SetField(sphere);
		maps[mapLoader.x + mapLoader.y * MAP_DIM] = newMap;
        mapLoader.needsToLoad = false;
	}
	mapLoaders.clear();
}

/* Returns an object representing the animation path */
void Level::GenPath()
{
    vector<vec3> vertices;
    vector<size_t> indices;
    
    GLuint index = 0;
    for (int i = 2; i < path.size() - 2; i++)
    {
        CMSpline *spline = path[i].spline;
        for (float u = 0; u < GRANULARITY; u++) {
            vec3 position = spline->evaluate3D(u / GRANULARITY);
            vertices.push_back(position);
            indices.push_back(index++);
        }
    }
    
    ArrayBuffer<vec3> ab(vertices);
    ElementArrayBuffer eab(indices);
    ModelBuffer mb(ab, eab);
    pathModel = new Model(mb, Material(), Bounds());
}

/* Precomputes splines between control points used to find
 positions of flyable objects as a function of time */
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

float Level::GetHeightAt(float x, float y) {
	float step = 1.f / MAP_DIM;
	int xi = (int) (x / step);
	//if (xi > 63) xi = 63;
	int yi = (int) (y / step);
	//if (yi > 63) yi = 63;
	float nx = (x - (step * xi)) * MAP_DIM;
	float ny = (y - (step * yi)) * MAP_DIM;
    if (maps[xi + yi * MAP_DIM])
        return maps[xi + yi * MAP_DIM]->GetHeightAt(nx, ny);
    else
        return 0;
}