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
    
    void PushMenu(Menu *other);
    void PopMenu();
    void PopSubMenu();
    void SetPrevious(Menu *p) { previous = p; }
    void SetNext(Menu *n) { next = n; }
    
    void Render();
    
    void SetWidth(int w) { width = w; }
    void SetHeight(int h) { height = h; }
    void SetSelectionActive(bool t) { selectionActive = t; }
    
    void HandleKey(int key, int action);
    void HandleChar(int character, int action);
    
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
