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
    void Resize(int w, int h);
    
    /** Sets the reticle position */
    void SetReticlePosition(glm::vec2 position);
    
    /** Gets the reticle position */
    glm::vec2 GetReticlePosition();

	/** will invalidate HUD elements */
	/** levelNum is the current numbered level the player is on */
	void Reset(int levelNum);
    
    bool reticleVisible;
    
private:
    
    /** Private shader program */
    Program *p;
    
    int width, height;
    float padding;
	int levelNum;
    
    /* Scene that HUD is monitoring */
    Scene *scene;
    
    FTPixmapFont *font;
    
    /** HUD elements */
	Minimap *minimap;
    Reticle *reticle;
	CDIndicator *thunderCD;
    
    /** Private rendering helpers */
    void RenderText();
    void RenderDynamicElements();
};
