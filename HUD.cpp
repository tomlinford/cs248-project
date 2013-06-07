#include "HUD.h"'
#include <sstream>

template<typename T>
string toString(T t) {
    stringstream s;
    s.precision(2);
    s << t;
    return s.str();
}

HUD::HUD()
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