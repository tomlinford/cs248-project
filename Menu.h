#pragma once

#include "gl.h"

#include <iostream>
#include <string>
#include <FTGL/ftgl.h>

typedef void (*menuFunc) (void *data);

class Menu
{
public:
    Menu(std::string options[], menuFunc functions[], int numItems);
    ~Menu();
    
    void Render();
    
    void SetWidth(int w) { width = w; }
    void SetHeight(int h) { height = h; }
    
    void HandleKey(int key, int action);
    
private:
    int selectedIndex;
    int numItems;
    int width, height;
    float padding;
    
    Menu *previous;
    
    menuFunc *functions;
    std::string *options;
    
    FTGLPixmapFont *font;
};
