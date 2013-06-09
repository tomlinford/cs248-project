#include "HUD.h"
#include <sstream>
#include <glm/glm.hpp>

using namespace::glm;

static Program *p = NULL;

template<typename T>
static string toString(T t) {
	stringstream s;
	s.precision(2);
	s << t;
	return s.str();
}

static float *createMinimap(Level *l) {
	vec3 *data = new vec3[MINIMAP_SIZE * MINIMAP_SIZE];
	for (int x = 0; x < MINIMAP_SIZE; x++) {
		for (int y = 0; y < MINIMAP_SIZE; y++) {
			float xf = (float) x /  (float) MINIMAP_SIZE;
			float yf = (float) y / (float) MINIMAP_SIZE;
			data[x + y * MINIMAP_SIZE] = vec3(0, l->GetHeightAt(xf, yf) * 0.7, l->GetHeightAt(xf, yf) * 0.9);
		}
	}
	return (float *) data;
}

HUD::HUD() : minimap(NULL), reticle(NULL)
{
    scene = NULL;
	font = new FTPixmapFont("01 Digitall.ttf");
	font->FaceSize(36);
	padding = 10;
}

HUD::~HUD()
{
	delete font;
	delete minimap;
    delete reticle;
}

void HUD::Render()
{
	if (!minimap) {
		Texture *tex = new Texture(MINIMAP_SIZE, MINIMAP_SIZE, GL_RGB,
			createMinimap(level));
		minimap = new Minimap(width - MINIMAP_SIZE - padding,
                              height - MINIMAP_SIZE - padding,
                              MINIMAP_SIZE, MINIMAP_SIZE,
                              width, height, tex, level);
	}
    if (!reticle) {
        Texture *tex = new Texture("reticle.bmp");
        reticle = new Reticle(width / 2 - 100 / 2,
                              height / 2 - 100 / 2,
                              100, 100,
                              width, height, tex);
    }

    if (!p) p = new Program("Shaders/hud.vert", "Shaders/hud.frag");
	minimap->Draw(*p, level);
    
    if (scene->player == PLAYER2) {
        reticle->Draw(*p, level);
    }

	glColor4f(1, 1, 1, 1);
	string level = "LEVEL: ";
	level.append(toString(0));
	glWindowPos2f(0, 0);
	font->Render(level.c_str(), -1, FTPoint(padding, height - padding - font->Ascender()));
    
    float h = 0;
    if (HUD::level->ship) {
        h = HUD::level->ship->GetHealth();
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