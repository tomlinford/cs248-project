#pragma once

#include <string>
#include <FTGL/ftgl.h>

#include "Level.h"
#include "Flyable.h"

class HUD
{
public:
    HUD();
    ~HUD();
    
    void LoadLevel(Level *l) { level = l; }
    void Render();
    
    void SetWidth(int w) { width = w; }
    void SetHeight(int h) { height = h; }
    
private:
    
    int width, height;
    float padding;
    Level *level;
    FTGLPixmapFont *font;
};
