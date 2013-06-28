#pragma once

#include "gl.h"

#include <iostream>
#include <assert.h>
#include <math.h>
#include <string>
#include <FTGL/ftgl.h>
#include <boost/timer/timer.hpp>

typedef void (*menuFunc) (void *data);

/** Represents a menu item; A menu item has a string label
 and a function to execute when it is selected */
class MenuItem
{
public:
    MenuItem() {}
    MenuItem(std::string l, menuFunc f = NULL) : label(l), func(f) {}
    
    virtual void HandleChar(int character, int action) {}
    virtual void HandleKey(int key, int action) {}
    
    menuFunc func;
    std::string label;
};

/** Represents a text field; Has a label and user-input text,
 but no associated execution function. */
class TextField : public MenuItem
{
public:
    TextField() {}
    TextField(std::string l, std::string d);
    
    void SetDefaultText(std::string d) { defaultText = d; }
    
    virtual void HandleChar(int key, int action);
    virtual void HandleKey(int key, int action);
    
    std::string GetCurrentText();
    
private:
    std::string original;
    std::string defaultText;
    std::string currentText;
};

/** Represents a simple key-based menu consisting of a series of
 items displayed vertically. */
class Menu
{
public:
    Menu(MenuItem *items[], int numItems, float fontsize = 72.0f);
    ~Menu();
    
    /* Pushes the given menu on top of the calling menu */
    void PushMenu(Menu *other);
    
    /* Pops the calling menu and shows its parent, if it
     has one */
    void PopMenu();
    
    /* Pops the calling menu's submenu, if it has one */
    void PopSubMenu();
    
    /* Changes the calling menu's parent to the specified 
     one */
    void SetPrevious(Menu *p) { previous = p; }
    
    /* Changes the calling menu's child to the specified 
     one */
    void SetNext(Menu *n) { next = n; }
    
    /* Draws the lowest child of the calling menu */
    void Render();
    
    /* Sets the dimensions of the menu */
    void SetDimensions(int w, int h) { width = w; height = h; }
    void SetSelectionActive(bool t) { selectionActive = t; }
    
    /* User input callbacks */
    void HandleKey(int key, int action);
    void HandleChar(int character, int action);
    
    /* Returns the current menu's lowest child */
    Menu *GetCurrentMenu();
    
    bool enlargeTitle;
    
private:
    
    void UpdateSelection(int dir);
    
    bool selectionActive;
    int selectedIndex;
    int numItems;
    int width, height;
    float padding;
    
    boost::timer::cpu_timer *timer;
    boost::timer::cpu_times times;
    
    Menu *next, *previous;
    MenuItem **items;
    
    FTGLPixmapFont *font;
    FTGLPixmapFont *largeFont;
};
