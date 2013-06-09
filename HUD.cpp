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

HUD::HUD() : minimap(NULL), reticle(NULL), scene(NULL)
{
	font = new FTPixmapFont("01 Digitall.ttf");
	font->FaceSize(36);
    
    p = new Program("Shaders/hud.vert", "Shaders/hud.frag");
    
    Texture *tex = new Texture("reticle.bmp");
    reticle = new Reticle(width / 2 - 100 / 2,
                          height / 2 - 100 / 2,
                          100, 100,
                          width, height, tex);
    
	padding = 10;
}

HUD::~HUD()
{
	delete font;
	delete minimap;
    delete reticle;
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
		h = scene->level->ship->GetHealth();
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
		minimap = new Minimap(width - MINIMAP_SIZE - padding,
                              height - MINIMAP_SIZE - padding,
                              MINIMAP_SIZE, MINIMAP_SIZE,
                              width, height,
                              scene->level->GetMinimap(MINIMAP_SIZE),
                              scene->level);
	}
    
	if (thunderCD == NULL)
		thunderCD = new CDIndicator(width - 240 - padding, padding, 36, 36, width, height, "Thunder Cooldown");
    
	minimap->Draw(*p, scene);
    
    if (scene->player == PLAYER2) {
        reticle->Draw(*p, scene);
        thunderCD->Draw(*p, scene);
    }
}

void HUD::SetHeight(int h) {
	height = h;
	if (minimap) {
		delete minimap;
		minimap = NULL;
	}
	if (thunderCD) {
		delete thunderCD;
		thunderCD = NULL;
	}
}

void HUD::SetWidth(int w) {
	width = w;
	if (minimap) {
		delete minimap;
		minimap = NULL;
	}
	if (thunderCD) {
		delete thunderCD;
		thunderCD = NULL;
	}
}