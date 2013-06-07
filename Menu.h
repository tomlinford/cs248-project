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
    
    void PushMenu(Menu *other);
    void PopMenu();
    void SetPrevious(Menu *p) { previous = p; }
    void SetNext(Menu *n) { next = n; }
    
    void Render();
    
    void SetWidth(int w) { width = w; }
    void SetHeight(int h) { height = h; }
    void SetSelectionActive(bool t) { selectionActive = t; }
    
    void HandleKey(int key, int action);
    
private:
    bool selectionActive;
    int selectedIndex;
    int numItems;
    int width, height;
    float padding;
    
    Menu *next, *previous;
    
    menuFunc *functions;
    std::string *options;
    
    FTGLPixmapFont *font;
};
