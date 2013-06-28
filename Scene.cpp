#include "Scene.h"

#include <sstream>
#include <boost/asio.hpp>

#include "Networking.h"
#include "Sound.h"

#define MAX_X 2.4
#define MAX_Y 1.8

using namespace::std;
using namespace::glm;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;

Scene::Scene() : particle_sys()
{
	player = PLAYER1;
	theta = phi = 0.0f;
	score = 0;
	totalScore = 0;
	levelNum = 0;

	finished = false;

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
	minimapTexture = NULL;

	Reset();

	// Spawn update thread
	updateThread = new thread(&Scene::Update, this);
}

Scene::~Scene()
{
	finished = true;
	updateThread->join();

	if (main)
		delete main;
	if (level)
		delete level;
	if (frustum)
		delete frustum;
}

void Scene::Reset()
{
	shipOffset = vec2(0);
	levelOver = false;
	gameOver = false;

	keyLeft = false;
	keyRight = false;
	keyDown = false;
	keyUp = false;
	mouseLeft = false;
	mouseRight = false;

	health = 10.f;

	lastLightning = -100.f;

	particle_sys.Clear();
}

void Scene::LoadLevel(Level *l, Player p)
{
	unique_lock<std::mutex> lock(mutex);

	player = p;

	Reset();

	if (level)
		delete level;
	level = l;
	level->sceneMutex = &mutex;

	level->Load();
	cond.notify_all();
	particle_sys.AddBulletCluster(level->ship->GetBulletCluster());
	for (int i = 0; i < level->objects.size(); i++) {
		Turret *turret = dynamic_cast<Turret *>(level->objects[i]);
        LaserShip *ship = dynamic_cast<LaserShip *>(level->objects[i]);
		if (turret) {
			particle_sys.AddBulletCluster(turret->GetBulletCluster());
		}
        else if (ship) {
            particle_sys.AddLaser(ship->GetLaser());
        }
	}
	if (timer) {
		delete timer;
		timer = new cpu_timer();
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

	if (minimapTexture)
		delete minimapTexture;
	minimapTexture = new Texture(width, height, GL_RGBA);

	if (fbo)
		delete fbo;
	fbo = new FBO(width, height);

	if (screen)
		delete screen;
	screen = new Screen();
}

/************** USER INPUT PROCESSING HERE **************/

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
	level->ship->SetOffset(shipOffset);
}

/** Adds bullets based on mouse presses. */
void Scene::HandleMouse(float elapsedSeconds)
{
	//float interval = 3 * (elapsedSeconds - lastTime);

	if (mouseLeft && level->ship) {
		vec3 selected = glm::unProject(vec3(512, 384, 1.0),
			view,
			projection,
			vec4(0, 0, 1024 ,768));
		vec3 velocity = normalize(selected - level->ship->GetPosition());
		level->ship->AddBullet(level->ship->GetPosition() + velocity, 20.0f * velocity);
		Networking::AddBullet(level->ship->GetPosition() + velocity, 20.0f * velocity);
		Sound::PlayLaser();
	}
	if (mouseRight && level->ship && lastTime - lastLightning > 5.f) {
		AddLightning(false);
		lastLightning = lastTime;
		Networking::AddLightning();
	}
}

/** Adds lightning effects */
void Scene::AddLightning(bool acquireLock)
{
	bool playThunder = false;

	if (acquireLock) {
		// lock will go out of scope otherwise
		lock_guard<std::mutex> lock(mutex);

		for (int i = 0; i < level->objects.size(); i++) {
			Object *obj = level->objects[i];
			if (glm::distance(obj->GetPosition(), level->ship->GetPosition()) < 30) {
				particle_sys.AddBolt(level->ship->GetPosition(), obj->GetPosition());
				particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
				delete obj;
				obj = NULL;
				level->objects.erase(level->objects.begin() + i--);
				playThunder = true;
			}
		}
	} else {
		for (int i = 0; i < level->objects.size(); i++) {
			Object *obj = level->objects[i];
			if (glm::distance(obj->GetPosition(), level->ship->GetPosition()) < 30) {
				particle_sys.AddBolt(level->ship->GetPosition(), obj->GetPosition());
				particle_sys.AddExplosionCluster(obj->GetPosition(), obj->GetColor());
				delete obj;
				obj = NULL;
				level->objects.erase(level->objects.begin() + i--);
				playThunder = true;
			}
		}
	}

	if (playThunder) Sound::PlayThunder();
}

/************** SCENE UDPATE METHODS HERE **************/

/** Updates objects by moving them to their new locations,
which is a function of the elapsed time. */
void Scene::UpdateObjects(float elapsedSeconds)
{
	// Update level objects
	level->Update(elapsedSeconds, lastTime);

	// Update light/camera
	lightPosition = level->ship->GetPosition();
	cameraPosition = level->ship->GetPosition() - 3.0f * level->ship->GetDirection();

	// Update particles
	particle_sys.Update(elapsedSeconds);
}

/** Determine if a collision has occurred - if so,
determine which objects to remove from the scene, and
whether to add particle effects at the location of
collision. */
void Scene::HandleCollisions(float elapsedSeconds)
{
	// Check collision between level objects
	level->HandleCollisions(elapsedSeconds, particle_sys, *frustum);

	// Update score
	if (player == PLAYER2)
		score = level->score;
	else
		health = level->ship->GetHealth();

	// restrict network updates
	static int count = 0;
	if (++count % 10 == 0) {
		if (player == PLAYER1 && level && level->ship)
			Networking::SetHealth(level->ship->GetHealth());
		else if (player == PLAYER2 && level)
			Networking::SetScore(level->score);
	}

	// Delete ship?
	if (level->ship && level->ship->GetHealth() < 0) {
		particle_sys.AddExplosionCluster(level->ship->GetPosition(), level->ship->GetColor());
		delete level->ship;
		level->ship = NULL;
		Networking::GameOver();
		gameOver = true;
	}
}

/** Updates the player views, which depends on the
ship position. */
void Scene::UpdateView(float elapsedSeconds)
{
	if (!level->ship)
		return;

	// View for player 1 (chase cam)
	if (player == PLAYER1)
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
	else
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

	// Minimap view (orthogonal overhead cam)
	vec3 position = level->ship->GetPosition();

	SetMinimapView(lookAt(position + vec3(0.0, 0.5, 0.0),
		position,
		vec3(0, 0, -1)));
}

/* Updates object positions in world based on elapsed
time and performs collision testing on a separate
thread */
void Scene::Update()
{
	while (!finished) {
		unique_lock<std::mutex> lock(mutex);

		// Wait until the level is ready
		while (!level || !level->Ready())
		{
			cond.wait(lock);
			if (timer)
				delete timer;
			timer = new cpu_timer();
			lastLightning = -100.f;
		}

		times = timer->elapsed();
		float elapsedSeconds = (float)times.wall / pow(10.f, 9.f);
		//elapsedSeconds += 70.f;

		if (!gameOver && elapsedSeconds > level->totalTime) {
			Networking::GameOver();
			gameOver = true;
			levelOver = true;
			totalScore += score;
		}
		if (gameOver) {
			continue;
		}

		// Handle player-specific input
		if (player == PLAYER2)
			HandleMouse(elapsedSeconds);
		HandleKeys(elapsedSeconds);
		UpdateObjects(elapsedSeconds);
		HandleCollisions(elapsedSeconds - lastTime);
		UpdateView(elapsedSeconds);

		lastTime = elapsedSeconds;
	}
}

/* Loads new objects via the main thread because OpenGL
calls are not thread safe */
void Scene::AddMissiles()
{
	if (!level->ship || player != PLAYER1)
		return;

	float interval = (float)timer->elapsed().wall / pow(10.f, 9.f) - lastTime;

	float size = level->objects.size();
	for (int i = 0; i < size; i++)
	{
		MissileShip *ship = dynamic_cast<MissileShip *>(level->objects[i]);
		if (!ship)
			continue;

		float distance = glm::distance(level->ship->GetPosition(), ship->GetPosition());
		if (distance < 20)
		{
			float lastFireTime = ship->GetLastFireTime();
			lastFireTime += interval;

			if (lastFireTime > ship->GetFiringRate()) {
				Networking::AddMissile(ship->GetOffset(), ship->GetTimeOffset());
				Missile *missile = new Missile("Models/missile.obj");
				missile->SetColor(vec3(1.0, 1.0, 1.0));
				missile->SetTimeOffset(ship->GetTimeOffset());
				missile->SetOffset(ship->GetOffset());
				level->objects.push_back(missile);
				ship->SetLastFireTime(0);
			}
			else {
				ship->SetLastFireTime(lastFireTime);
			}
		}
	}
}

/************** SCENE RENDERING METHODS HERE **************/

void Scene::RenderObjects(DrawMode mode)
{
	// Draw sphere
	if (level->sphere && frustum->Contains(*level->sphere)) {
		main->SetUniform("hasField", true);
		main->SetUniform("fieldRadius", level->sphere->GetScale());
		main->SetUniform("fieldPosition", level->sphere->GetPosition());
		level->sphere->Draw(*main, viewProjection, cameraPosition, mode);
	}

	// Draw objects
	for (int i = 0; i < level->objects.size(); i++) {
		Object *obj = level->objects[i];
		if (frustum->Contains(*obj))
			obj->Draw(*main, viewProjection, cameraPosition, mode);
	}

	// Draw ship
	if (level->ship && frustum->Contains(*level->ship))
		level->ship->Draw(*main, viewProjection, cameraPosition, mode);

	// Draw particles
	particle_sys.Draw(*main, viewProjection, cameraPosition, mode);
}

void Scene::RenderGlowMap()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw maps
	for (Map *map : level->maps) {
		if (!map || !frustum->Contains(*map)) continue;
		map->Draw(viewProjection, cameraPosition, lightPosition, GLOW);
	}

	main->Use();
	main->SetUniform("attenuate", true);
	main->SetUniform("hasField", false);
	main->SetUniform("lightPosition", lightPosition);
	main->SetUniform("cameraPosition", cameraPosition);

	RenderObjects(GLOW);

	main->Unuse();

	glBindTexture(GL_TEXTURE_2D, glowTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, glowTexture->GetFormat(), 0, 0, glowTexture->GetWidth(), glowTexture->GetHeight(), 0);
}

void Scene::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Maps have their own shader program for vertex displacement
	for (Map *map : level->maps) {
		if (!map || !frustum->Contains(*map)) continue;
		map->Draw(viewProjection, cameraPosition, lightPosition, NORMAL);
	}

	main->Use();
	main->SetUniform("attenuate", true);
	main->SetUniform("hasField", false);
	main->SetUniform("lightPosition", lightPosition);
	main->SetUniform("cameraPosition", cameraPosition);

	RenderObjects(NORMAL);

	main->Unuse();

	glBindTexture(GL_TEXTURE_2D, sceneTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, sceneTexture->GetFormat(), 0, 0, sceneTexture->GetWidth(), sceneTexture->GetHeight(), 0);

	glBindTexture(GL_TEXTURE_2D, depthTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, depthTexture->GetFormat(), 0, 0, depthTexture->GetWidth(), depthTexture->GetHeight(), 0);
}

void Scene::RenderMinimap()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	mat4 minimapViewProjection = minimapProjection * minimapView;

	// Maps have their own shader program for vertex displacement
	for (Map *map : level->maps) {
		if (!map || !frustum->Contains(*map)) continue;
		map->Draw(viewProjection, cameraPosition, lightPosition, MINIMAP);
	}

	main->Use();
	main->SetUniform("attenuate", true);
	main->SetUniform("lightPosition", lightPosition);
	main->SetUniform("cameraPosition", cameraPosition);

	RenderObjects(MINIMAP);

	// Draw level path
	main->SetMVP(minimapViewProjection);
	main->SetUniform("baseColor", vec3(1.0));
	main->SetUniform("illum", 0);
	main->SetUniform("attenuate", false);

	glLineWidth(10.0f);
	level->pathModel->Draw(*main, GL_LINE_STRIP);

	main->Unuse();

	glBindTexture(GL_TEXTURE_2D, minimapTexture->GetID());
	glCopyTexImage2D(GL_TEXTURE_2D, 0, minimapTexture->GetFormat(), 0, 0, minimapTexture->GetWidth(), minimapTexture->GetHeight(), 0);

	glEnable(GL_DEPTH_TEST);
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
	particle_sys.Draw(*velocity, viewProjection, cameraPosition, NORMAL);

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

	AddMissiles();

	viewProjection = projection * view;

	RenderGlowMap();
	RenderScene();
	//RenderMinimap();
	RenderVelocityTexture();
	PostProcess();

	prevViewProjection = viewProjection;
}
