#include "HUD.h"'
#include <sstream>
#include <glm/glm.hpp>

using namespace glm;

#define MINIMAP_SIZE 256

template<typename T>
static string toString(T t) {
	stringstream s;
	s.precision(2);
	s << t;
	return s.str();
}

static float *createMinimap(Level *l) {
	vec3 *data = new vec3[MINIMAP_SIZE * MINIMAP_SIZE];
	for (int i = 0; i < MINIMAP_SIZE * MINIMAP_SIZE; i++) {
		data[i] = vec3(0, 0, 1);
	}
	return (float *) data;
}

HUD::HUD() : minimap(NULL)
{
	font = new FTPixmapFont("01 Digitall.ttf");
	font->FaceSize(36);
	padding = 10;
}

HUD::~HUD()
{
	delete font;
}

void HUD::Render()
{
	if (minimap == NULL)
		minimap = new Texture(MINIMAP_SIZE, MINIMAP_SIZE, GL_RGB, createMinimap(level));

	//glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho (0, width, height, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	glUseProgram(0);
	//minimap->Bind();
	//glBindTexture(GL_TEXTURE_2D, minimap->GetID());
	glBegin(GL_QUADS); {
		int x = width - MINIMAP_SIZE - 5;
		int y = height - MINIMAP_SIZE - 5;
		glColor3f(0, 0, 1);
		//glTexCoord2f(0, 0);
		glVertex2f(x, y);
		//glTexCoord2f(0, 1);
		glVertex2f(x, y + MINIMAP_SIZE);
		//glTexCoord2f(1, 1);
		glVertex2f(x + MINIMAP_SIZE, y + MINIMAP_SIZE);
		//glTexCoord2f(1, 0);
		glVertex2f(x + MINIMAP_SIZE, y);
	} glEnd();

	glColor4f(1, 1, 1, 1);
	string level = "LEVEL: ";
	level.append(toString(0));
	glWindowPos2f(0, 0);
	font->Render(level.c_str(), -1, FTPoint(padding, height - padding - font->Ascender()));

	float h = HUD::level->ship->GetHealth();
	if (h < 0) h = 0;
	glColor4f(1 - h / 10, h / 10, 0, 1);
	string health = "HEALTH: ";
	health.append(toString(h));
	FTBBox box = font->BBox(health.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
	glWindowPos2f(0, 0);
	font->Render(health.c_str(), -1, FTPoint(width - 2 * padding - box.Upper().X(), height - padding - font->Ascender()));

	glColor4f(1, 1, 1, 1);
	string score = "SCORE: ";
	score.append(toString(0));
	glWindowPos2f(0, 0);
	font->Render(score.c_str(), -1, FTPoint(5, 5));
}