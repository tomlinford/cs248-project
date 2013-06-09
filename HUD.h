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
    
    /** Associates this HUD with the given scene */
    void LoadScene(Scene *s) { scene = s; }
    
    /** Main rendering function */
    void Render();
    
    /** Call when window is resized to reposition
     HUD elements */
    void SetWidth(int w);
    void SetHeight(int h);
    
private:
    
    /** Private shader program */
    Program *p;
    
    int width, height;
    float padding;
    
    Scene *scene;
    
    FTPixmapFont *font;
    
	Minimap *minimap;
    Reticle *reticle;
	CDIndicator *thunderCD;
    
    /** Private rendering helpers */
    void RenderText();
    void RenderDynamicElements();
};
