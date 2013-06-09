#include "HUD.h"
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace::glm;

template<typename T>
static string toString(T t) {
	stringstream s;
	s.precision(2);
	s << t;
	return s.str();
}

HUD::HUD() : minimap(NULL), reticle(NULL), thunderCD(NULL), scene(NULL)
{
	font = new FTPixmapFont("01 Digitall.ttf");
	font->FaceSize(36);

	p = new Program("Shaders/hud.vert", "Shaders/hud.frag");

	padding = 10;
}

HUD::~HUD()
{
	delete font;
	delete minimap;
	delete reticle;
	delete thunderCD;
	delete p;
}

void HUD::Render()
{
	if (!scene || !scene->level)
		return;

	RenderDynamicElements();
	RenderText();
}

void HUD::RenderText()
{
	glColor4f(1, 1, 1, 1);
	string level = "LEVEL: ";
	level.append(toString(0));
	glWindowPos2f(0, 0);
	font->Render(level.c_str(), -1, FTPoint(padding, height - padding - font->Ascender()));

	float h = 0;
	if (scene->level->ship) {
		h = scene->health;
		if (h < 0)
			h = 0;
	}

	glColor4f(1 - h / 10, h / 10, 0, 1);
	string health = "HEALTH: ";
	health.append(toString(h));
	FTBBox box = font->BBox(health.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
	glWindowPos2f(0, 0);
	font->Render(health.c_str(), -1, FTPoint(width - 2 * padding - box.Upper().X(), height - padding - font->Ascender()));

	glColor4f(1, 1, 1, 1);
	string score = "SCORE: ";
	if (scene) score.append(toString(scene->score));
	box = font->BBox(score.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
	glWindowPos2f(0, 0);
	font->Render(score.c_str(), -1, FTPoint(width / 2 - box.Upper().X() / 2, height - padding - font->Ascender()));
}

void HUD::RenderDynamicElements()
{
	if (!minimap) {
		minimap = new Minimap(MINIMAP_SIZE, MINIMAP_SIZE,
			scene->level->GetMinimap(MINIMAP_SIZE),
			scene->level);
		minimap->SetProjection(width, height);
		minimap->SetPosition(vec3(width / 2 - padding - MINIMAP_SIZE / 2,
			height / 2 - padding - MINIMAP_SIZE / 2,
			0.0f));
	}

	if (!reticle) {
		Texture *tex = new Texture("reticle.bmp");
		reticle = new Reticle(100, 100, tex);
		reticle->SetProjection(width, height);
		reticle->SetPosition(vec3(0.0f));
	}

	if (!thunderCD) {
		thunderCD = new CDIndicator(36, 36, "Thunder Cooldown");
		thunderCD->SetProjection(width, height);
		thunderCD->SetPosition(vec3(width / 2 - 276 - padding,
			-height / 2 + padding + 18,
			0.0f));
	}

	minimap->Draw(*p, scene);

	if (scene->player == PLAYER2) {
		reticle->Draw(*p, scene);
		thunderCD->Draw(*p, scene);
	}
}

void HUD::Resize(int w, int h) {
	width = w;
	height = h;

	if (minimap) {
		minimap->SetProjection(width, height);
		reticle->SetProjection(width, height);
		thunderCD->SetProjection(width, height);

		minimap->SetPosition(vec3(width / 2 - padding - MINIMAP_SIZE / 2,
			height / 2 - padding - MINIMAP_SIZE / 2,
			0.0f));
		thunderCD->SetPosition(vec3(width / 2 - 276 - padding,
			-height / 2 + padding + 18,
			0.0f));
	}
}