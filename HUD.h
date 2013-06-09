#pragma once

#include <string>
#include <FTGL/ftgl.h>

#include "Scene.h"
#include "Level.h"
#include "Flyable.h"
#include "HudElement.h"

class HUD
{
public:
    HUD();
    ~HUD();
    
    void LoadScene(Scene *s) { scene = s; }
    void LoadLevel(Level *l) { level = l; }
    void Render();
    
    void SetWidth(int w) {
		width = w;
		if (minimap) {
			delete minimap;
			minimap = NULL;
		}
	}
    void SetHeight(int h) {
		height = h;
		if (minimap) {
			delete minimap;
			minimap = NULL;
		}
	}
    
private:
    
    int width, height;
    float padding;
    
    Scene *scene;
    Level *level;
    
    FTPixmapFont *font;
    
	Minimap *minimap;
    Reticle *reticle;
};
