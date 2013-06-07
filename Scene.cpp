#include "Scene.h"

#include <sstream>
#include <thread>
#include <boost/asio.hpp>

#include "Networking.h"

#define MAX_X 2.4
#define MAX_Y 1.8

using namespace::std;
using namespace::glm;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;

static int count;

Scene::Scene(Player p) : particle_sys()
{
	player = p;
	theta = phi = 0.0f;

	frustum = new Frustum();
	main = new Program("Shaders/main.vert", "Shaders/main.frag");
	screenProgram = new Program("Shaders/quad.vert", "Shaders/quad.frag");
	vblur = new Program("Shaders/vblur.vert", "Shaders/blur.frag");
	hblur = new Program("Shaders/hblur.vert", "Shaders/blur.frag");
	mblur = new Program("Shaders/mblur.vert", "Shaders/mblur.frag");
	velocity = new Program("Shaders/velocity.vert", "Shaders/velocity.frag");

	level = NULL;
	timer = NULL;
	fbo = NULL;
	screen = NULL;
	depthTexture = NULL;
	glowTexture = NULL;
	vblurTexture = NULL;
	hblurTexture = NULL;
	mblurTexture = NULL;
	velocityTexture = NULL;
	combinedTexture = NULL;
	sceneTexture = NULL;

	keyLeft = false;
	keyRight = false;
	keyDown = false;
	keyUp = false;
	mouseLeft = false;
	mouseRight = false;

	// Spawn update thread
	thread updateThread(&Scene::Update, this);
	updateThread.detach();
}

Scene::~Scene()
{
	if (main)
		delete main;
	if (level)
		delete level;
	if (frustum)
		delete frustum;
}

void Scene::LoadLevel(Level *l)
{
	level = l;
	level->Load();
    cond.notify_all();
    particle_sys.AddBulletCluster(level->ship->GetBulletCluster());
    for (int i = 0; i < level->objects.size(); i++) {
        Turret *turret = dynamic_cast<Turret *>(level->objects[i]);
        if (turret) {
            particle_sys.AddBulletCluster(turret->GetBulletCluster());
        }
    }
}


void Scene::UpdateFBO(GLuint width, GLuint height)
{
	unique_lock<std::mutex> lock(mutex);

	if (depthTexture)
		delete depthTexture;
	depthTexture = new Texture(width, height, GL_DEPTH_COMPONENT);

	if (glowTexture)
		delete glowTexture;
	glowTexture = new Texture(width, height, GL_RGBA);

	if (hblurTexture)
		delete hblurTexture;
	hblurTexture = new Texture(width, height, GL_RGBA);

	if (vblurTexture)
		delete vblurTexture;
	vblurTexture = new Texture(width, height, GL_RGBA);

	if (mblurTexture)
		delete mblurTexture;
	mblurTexture = new Texture(width, height, GL_RGBA);

	if (velocityTexture)
		delete velocityTexture;
	velocityTexture = new Texture(width, height, GL_RGBA);

	if (combinedTexture)
		delete combinedTexture;
	combinedTexture = new Texture(width, height, GL_RGBA);

	if (sceneTexture)
		delete sceneTexture;
	sceneTexture = new Texture(width, height, GL_RGBA);

	if (fbo)
		delete fbo;
	fbo = new FBO(width, height);

	if (screen)
		delete screen;
	screen = new Screen();
}

/* Scene update functions below */

/** Captures ship offset based on key presses. */
void Scene::HandleKeys(float elapsedSeconds)
{
	float interval = 3 * (elapsedSeconds - lastTime);
	if (level->sphere &&
		level->ship &&
		glm::distance(level->ship->GetPosition(), level->sphere->GetPosition()) < level->sphere->GetScale())
	{
		interval *= -1.0;
	}

	if (keyLeft)
		shipOffset.x -= interval;
	if (keyRight)
		shipOffset.x += interval;
	if (keyUp)
		shipOffset.y += interval;
	if (keyDown)
		shipOffset.y -= interval;

	shipOffset = glm::clamp(shipOffset, vec2(-MAX_X, -MAX_Y), vec2(MAX_X, MAX_Y));
}

/** Adds bullets based on mouse presses. */
void Scene::HandleMouse(float elapsedSeconds)
{
	float interval = 3 * (elapsedSeconds - lastTime);

	if (mouseLeft && level->ship) {
		vec3 selected = glm::unProject(vec3(512, 384, 1.0),
			view,
			projection,
			vec4(0, 0, 1024 ,768));
		vec3 velocity = normalize(selected - level->ship->GetPosition());
		level->ship->AddBullet(level->ship->GetPosition() + velocity, 20.0f * velocity);
		Networking::AddBullet(level->ship->GetPosition() + velocity, 20.0f * velocity);
	}
	if (mouseRight && level->ship) {
		AddLightning();
		Networking::AddLightning();
	}
}

/** Adds lightning effects */
void Scene::AddLightning() {
	for (int i = 0; i < level->objects.size(); i++) {
		Object *obj = level->objects[i];
		if (glm::distance(obj->GetPosition(), level->ship->GetPosition()) < 30) {
			particle_sys.AddBolt(level->ship->GetPosition(), obj->GetPosition());
			particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
			delete obj;
			obj = NULL;
			level->objects.erase(level->objects.begin() + i--);
		}
	}
}

/** Updates objects by moving them to their new locations,
which is a function of the elapsed time. */
void Scene::UpdateObjects(float elapsedSeconds)
{
	// Compute ship position along path
	if (level->ship)
	{
		vec3 shipDirection = level->GetDirection(FORWARD, elapsedSeconds);
		vec3 shipPosition = level->GetPosition(FORWARD, elapsedSeconds);

		// Update light/camera
		lightPosition = shipPosition;
		cameraPosition = shipPosition - 3.0f * shipDirection;

		// Update ship position
		// Important: Note offset and direction must be set before
		// position
		level->ship->SetOffset(shipOffset);
		level->ship->SetDirection(shipDirection);
		level->ship->SetPosition(shipPosition);
	}
    
    // Update sphere
    if (level->sphere) {
        level->sphere->SetScale(75 + 7 * sin(elapsedSeconds));
    }

	// Update other objects
	for (int i = 0; i < level->objects.size(); i++)
	{
		Object *obj = level->objects[i];
        Flyable *flyable = dynamic_cast<Flyable *>(obj);
        Missile *missile = dynamic_cast<Missile *>(obj);
        Turret *turret = dynamic_cast<Turret *>(obj);
        
        if (missile && level->ship)
        {
            vec2 missileOffset = missile->GetOffset();
            vec2 shipOffset = level->ship->GetOffset();
            shipOffset.x *= -1;
            
            vec2 dir = shipOffset - missileOffset;
            vec2 offset = 0.01f * dir;
            missile->SetOffset(missileOffset + offset);
        }
        if (flyable)
        {
            vec3 direction = level->GetDirection(BACKWARD, elapsedSeconds + flyable->GetTimeOffset());
            vec3 position = level->GetPosition(BACKWARD, elapsedSeconds + flyable->GetTimeOffset());
            
            // Remove objects off screen
            if (position.x - 0.0 < 0.0001 &&
                position.y - 0.0 < 0.0001 &&
                position.z - 0.0 < 0.0001)
            {
				level->objects.erase(level->objects.begin() + i--);
            }
            else
            {
                flyable->SetDirection(direction);
                flyable->SetPosition(position);
            }
        }
        if (turret && level->ship) {
            vec3 turretPos = turret->GetPosition();
            vec3 shipPos = level->ship->GetPosition();
            float distance = fabs(glm::distance(shipPos, turretPos));
            if (distance < 30) {
                vec3 dir = normalize(shipPos - turretPos);
                turret->AddBullet(turretPos, 20.0f * dir);
            }
        }
    }
    
    // Update particles
    particle_sys.Update(elapsedSeconds);
}

/** Determine if a collision has occurred - if so,
determine which objects to remove from the scene, and
whether to add particle effects at the location of
collision. */
void Scene::HandleCollisions(float elapsedSeconds)
{
	// Check map collision
	for (int i = 0; i < level->maps.size(); i++)
	{
		Map *map = level->maps[i];

		// If the map is not in the frustum,
		// skip it for now
		if (!frustum->Contains(*map))
			continue;

		// Check ship intersections
		if (level->ship && map->Intersects(*level->ship)) {
			particle_sys.AddExplosionCluster(level->ship->GetPosition(), map->GetColor());
			level->ship->AddDamage(0.001 * elapsedSeconds);
		}
	}

	// Check object collisions
	for (int i = 0; i < level->objects.size(); i++)
	{
		Object *obj = level->objects[i];
		Missile *missile = dynamic_cast<Missile *>(obj);
        Turret *turret = dynamic_cast<Turret *>(obj);

		// If the object is not in the frustum,
		// skip it for now
		if (!frustum->Contains(*obj))
			continue;

		// Check object-bullet intersections
		if (particle_sys.Intersects(obj)) {
            if (turret) {
                turret->AddDamage(0.5);
                if (turret->GetHealth() < 0) {
                    particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
                    delete obj;
                    obj = NULL;
                    level->objects.erase(level->objects.begin() + i--);
                    continue;
                }
            }
            else {
                particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
                delete obj;
                obj = NULL;
                level->objects.erase(level->objects.begin() + i--);
                continue;
            }
		}

		// Check object-ship intersections
		if (level->ship && level->ship->Intersects(*obj))
		{
			if (missile)
			{
				particle_sys.AddExplosionCluster(level->ship->GetPosition(), missile->GetColor());
				level->ship->AddDamage(0.5);
			}
            else {
                particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
                level->ship->AddDamage(0.2);
                delete obj;
                obj = NULL;
                level->objects.erase(level->objects.begin() + i--);
                continue;
            }
		}
	}
    
    // Delete ship?
    if (level->ship && level->ship->GetHealth() < 0) {
        /*particle_sys.AddExplosionCluster(level->ship->GetPosition(), level->ship->GetColor());
        delete level->ship;
        level->ship = NULL;*/
    }
}

/** Updates the player views, which depends on the
ship position. */
void Scene::UpdateView(float elapsedSeconds)
{
	// View for player 1 (chase cam)
	if (player == PLAYER1 && level->ship)
	{
		// Compute ship position along path
		vec3 position = level->GetPosition(FORWARD, elapsedSeconds);
		quat orientation = level->ship->GetOrientation();
		vec3 direction = orientation * vec3(0, 0, -1);
		vec3 up = orientation * vec3(0, 1, 0);

		SetView(lookAt(position - 3.0f * direction,
			position + direction,
			up));

		frustum->LoadCam(position - 3.0f * direction,
			position + direction,
			up);
	}
	// View for player 2 (on board cam)
	else if (level->ship)
	{
		// Compute view vectors
		vec3 position = level->ship->GetPosition();
		quat orientation = level->ship->GetOrientation() * fquat(vec3(phi, theta, 0));
		vec3 direction = orientation * vec3(0, 0, -1);
		vec3 up = orientation * vec3(0, 1, 0);

		SetView(lookAt(position + 0.3f * up,
			position + 0.3f * up + direction,
			up));

		frustum->LoadCam(position - 3.0f * direction,
			position + direction,
			up);
	}
}

/* Updates object positions in world based on elapsed
time and performs collision testing on a separate
thread */
void Scene::Update()
{
	while (true) {
		unique_lock<std::mutex> lock(mutex);

		// Wait until the level is ready
		while (!level || !level->Ready())
		{
			cond.wait(lock);
			if (timer)
				delete timer;
			timer = new cpu_timer();
		}

		times = timer->elapsed();
		float elapsedSeconds = (float)times.wall / pow(10.f, 9.f);

		// Handle player-specific input
		if (player == PLAYER1)
			HandleKeys(elapsedSeconds);
		else {
			HandleKeys(elapsedSeconds);
			HandleMouse(elapsedSeconds);
		}

		UpdateObjects(elapsedSeconds);
		HandleCollisions(elapsedSeconds);
		UpdateView(elapsedSeconds);

		lastTime = elapsedSeconds;
	}
}

/* Loads new objects via the main thread because OpenGL
calls are not thread safe */
void Scene::LoadNewObjects()
{
	float size = level->objects.size();
	for (int i = 0; i < size; i++)
	{
		Ship *ship = dynamic_cast<Ship *>(level->objects[i]);
		if (ship &&
			level->ship &&
			::count % 30 == 0 &&
			glm::distance(level->ship->GetPosition(), ship->GetPosition()) < 20)
		{
			Missile *missile = new Missile("Models/missile.obj");
			missile->SetColor(vec3(1.0, 1.0, 1.0));
			missile->SetTimeOffset(ship->GetTimeOffset());
			missile->SetOffset(ship->GetOffset());
			level->objects.push_back(missile);
		}
	}
}

/* Rendering functions below */

void Scene::RenderGlowMap()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Maps have their own shader program for vertex displacement
	level->DrawMap(projection * view, cameraPosition, lightPosition, *frustum, true);

	main->Use();
	main->SetUniform("illum", 0);
    main->SetUniform("attenuate", true);
	main->SetUniform("lightPosition", lightPosition);
	main->SetUniform("cameraPosition", cameraPosition);

	if (level->sphere && frustum->Contains(*level->sphere))
	{
		level->sphere->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
	}

	// Draw ship
	if (level->ship && frustum->Contains(*level->ship))
	{
		level->ship->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
	}

	// Draw objects in scene
	for (int i = 0; i < level->objects.size(); i++)
	{
		Object *obj = level->objects[i];
		if (frustum->Contains(*obj))
			obj->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
	}

	// Draw particles
    main->SetUniform("attenuate", false);
	particle_sys.Draw(*main, viewProjection, cameraPosition, true);

	main->Unuse();

	glBindTexture(GL_TEXTURE_2D, glowTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, glowTexture->GetFormat(), 0, 0, glowTexture->GetWidth(), glowTexture->GetHeight(), 0);
}

void Scene::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Maps have their own shader program for vertex displacement
	level->DrawMap(viewProjection, cameraPosition, lightPosition, *frustum, false);

	main->Use();
    main->SetUniform("attenuate", true);
	main->SetUniform("lightPosition", lightPosition);
	main->SetUniform("cameraPosition", cameraPosition);

	// Draw sphere
	if (level->sphere && frustum->Contains(*level->sphere)) {
		main->SetUniform("illum", 1);
		level->sphere->Draw(*main, viewProjection, cameraPosition);
		main->SetUniform("illum", 0);
		level->sphere->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
		main->SetUniform("fieldPosition", level->sphere->GetPosition());
	}

	// Draw ship
	if (level->ship && frustum->Contains(*level->ship))
	{
		main->SetUniform("illum", 1);
		level->ship->Draw(*main, viewProjection, cameraPosition);
		main->SetUniform("illum", 0);
		level->ship->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
	}

	// Draw objects in scene
	for (int i = 0; i < level->objects.size(); i++)
	{
		Object *obj = level->objects[i];
		if (frustum->Contains(*obj))
		{
			main->SetUniform("illum", 1);
			obj->Draw(*main, viewProjection, cameraPosition);
			main->SetUniform("illum", 0);
			obj->Draw(*main, viewProjection, cameraPosition, GL_LINE_LOOP);
		}
	}

	// Draw particles
    main->SetUniform("attenuate", false);
	particle_sys.Draw(*main, viewProjection, cameraPosition, false);

	main->Unuse();

	glBindTexture(GL_TEXTURE_2D, sceneTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, sceneTexture->GetFormat(), 0, 0, sceneTexture->GetWidth(), sceneTexture->GetHeight(), 0);

	glBindTexture(GL_TEXTURE_2D, depthTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, depthTexture->GetFormat(), 0, 0, depthTexture->GetWidth(), depthTexture->GetHeight(), 0);
}

void Scene::RenderVelocityTexture()
{
	fbo->Use();
	fbo->SetColorTexture(velocityTexture, GL_COLOR_ATTACHMENT2);
	fbo->SetDrawTarget(GL_COLOR_ATTACHMENT2);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	velocity->Use();
	velocity->SetUniform("previousVP", prevViewProjection);
	velocity->SetUniform("currentVP", viewProjection);

	// Draw particles
	particle_sys.Draw(*velocity, viewProjection, cameraPosition, false);

	velocity->Unuse();
}

void Scene::PostProcess()
{
	// Motion blur
	fbo->SetColorTexture(mblurTexture, GL_COLOR_ATTACHMENT3);
	fbo->SetDrawTarget(GL_COLOR_ATTACHMENT3);
	mblur->Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mblur->SetUniform("sceneColorTexture", sceneTexture, GL_TEXTURE0);
	mblur->SetUniform("sceneDepthTexture", depthTexture, GL_TEXTURE1);
	mblur->SetUniform("velocityTexture", velocityTexture, GL_TEXTURE2);
	screen->Draw(*mblur);
	mblur->Unuse();

	// Horizontal blur
	fbo->SetColorTexture(hblurTexture, GL_COLOR_ATTACHMENT0);
	fbo->SetDrawTarget(GL_COLOR_ATTACHMENT0);
	hblur->Use();
	hblur->SetUniform("colorTexture", glowTexture, GL_TEXTURE0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	screen->Draw(*hblur);
	hblur->Unuse();

	// Vertical blur
	fbo->SetColorTexture(vblurTexture, GL_COLOR_ATTACHMENT5);
	fbo->SetDrawTarget(GL_COLOR_ATTACHMENT5);
	vblur->Use();
	vblur->SetUniform("colorTexture", hblurTexture, GL_TEXTURE0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	screen->Draw(*vblur);
	vblur->Unuse();

	fbo->Unuse();

	// Draw to screen
	screenProgram->Use();
	screenProgram->SetUniform("scene", mblurTexture, GL_TEXTURE0);
	screenProgram->SetUniform("postProcess", vblurTexture, GL_TEXTURE1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	screen->Draw(*screenProgram);
	screenProgram->Unuse();
}

void Scene::Render()
{
	if (!timer) return;
	unique_lock<std::mutex> lock(mutex);

	LoadNewObjects();

	::count++;

	viewProjection = projection * view;

	RenderGlowMap();
	RenderScene();
	RenderVelocityTexture();
	PostProcess();

	prevViewProjection = viewProjection;
}
