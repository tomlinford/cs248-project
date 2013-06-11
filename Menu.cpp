

#include "Menu.h"
#include "Sound.h"

using namespace::std;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;

TextField::TextField(string l, string d) :
    MenuItem(l + d, NULL), original(l), defaultText(d)
{
}

void TextField::HandleChar(int character, int action)
{
    currentText += toupper((char)character);
    label = original + currentText;
}

void TextField::HandleKey(int key, int action)
{
    switch(key) {
        case GLFW_KEY_BACKSPACE:
            currentText = currentText.substr(0, currentText.size() - 1);
            if (currentText.empty()) {
                label = original + defaultText;
            }
            else {
                label = original + currentText;
            }
            break;
        default:
            break;
    }
}

std::string TextField::GetCurrentText()
{
    if (currentText.empty())
        return defaultText;
    return currentText;
}

Menu::Menu(MenuItem *it[], int i, float fontsize)
{
    items = it;
    numItems = i;
    
    next = NULL;
    previous = NULL;
    
    selectedIndex = -1;
    for (int j = 0; j < numItems; j++) {
        if (items[j]->func) {
            selectedIndex = j;
            break;
        }
    }
    selectionActive = false;
    
    padding = 20.0f;
    
    enlargeTitle = false;
    
    font = new FTGLPixmapFont("01 Digitall.ttf");
	font->FaceSize(fontsize);
    
    largeFont = new FTGLPixmapFont("01 Digitall.ttf");
	largeFont->FaceSize(fontsize + 20);
    
    timer = new cpu_timer();
}

Menu::~Menu()
{
    delete font;
}

void Menu::PushMenu(Menu *other)
{
    assert(this != other);
    next = other;
    other->SetPrevious(this);
    Sound::PlayBack();
}

void Menu::PopMenu()
{
    if (previous) {
        previous->SetNext(NULL);
        previous->SetSelectionActive(false);
        Sound::PlayBack();
    }
}

void Menu::PopSubMenu()
{
    if (next)
        next->PopMenu();
}

void Menu::HandleChar(int character, int action)
{
    if (next) {
        next->HandleChar(character, action);
        return;
    }
    
    if (selectionActive) {
        items[selectedIndex]->HandleChar(character, action);
    }
}

void Menu::UpdateSelection(int dir)
{
    if (selectedIndex == -1)
        return;
    
    TextField *f = NULL;
    int orig_index = selectedIndex;
    do {
        selectedIndex += dir;
        if (selectedIndex > numItems - 1)
            selectedIndex = 0;
        else if (selectedIndex < 0)
            selectedIndex = numItems - 1;
        f = dynamic_cast<TextField *>(items[selectedIndex]);
    } while (!items[selectedIndex]->func && !f && selectedIndex != orig_index);
    
    Sound::PlaySelect();
}

void Menu::HandleKey(int key, int action)
{
    if (next) {
        next->HandleKey(key, action);
        return;
    }
    
    if (action == GLFW_RELEASE) {
        return;
    }
    if (selectedIndex != -1 && selectionActive) {
        items[selectedIndex]->HandleKey(key, action);
    }
    
    switch(key) {
        case GLFW_KEY_UP:
            if (selectedIndex != -1 && selectionActive)
                selectionActive = false;
            UpdateSelection(-1);
            break;
        case GLFW_KEY_DOWN:
            if (selectedIndex != -1 && selectionActive)
                selectionActive = false;
            UpdateSelection(1);
            break;
        case GLFW_KEY_ENTER:
        {
            if (selectedIndex != -1)
                selectionActive = true;
            if (selectedIndex != -1 &&
                items[selectedIndex]->func)
                items[selectedIndex]->func(NULL);
            break;
        }
        case GLFW_KEY_ESC:
        {
            if (selectionActive)
                selectionActive = false;
            else if (previous)
                PopMenu();
            break;
        }
        default:
            break;
    }
}

Menu* Menu::GetCurrentMenu()
{
    if (next)
        return next->GetCurrentMenu();
    return this;
}

void Menu::Render()
{
    if (next) {
        next->Render();
        return;
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    times = timer->elapsed();
    float elapsedSeconds = (float)times.wall / pow(10.f, 9.f);
    
    float itemHeight = font->Ascender() * numItems + padding * (numItems - 1);
    float start = height / 2.0f + itemHeight / 2.0f;
    
    for (int i = 0; i < numItems; i++) {
        
        if (i == selectedIndex) {
            float k = 1.0;
            if (!selectionActive)
                k = sin(elapsedSeconds * 5) + 1;
            glColor4f(0.0, 0.7, 0.9, k * 1.0);
        }
        else {
            if (i == 0 && enlargeTitle) {
                glColor4f(1.0, 1.0, 1.0, 1.0);
            }
            else {
                glColor4f(0.5, 0.5, 0.5, 1.0);
            }
        }
        glWindowPos2f(0,0);
        
        string item = items[i]->label;
        if (enlargeTitle && i == 0) {
            FTBBox box = largeFont->BBox(item.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
            largeFont->Render(item.c_str(),
                              -1,
                              FTPoint(width / 2.0f - box.Upper().X() / 2.0f,
                                      start - i * (padding + largeFont->Ascender())));
        }
        else {
            FTBBox box = font->BBox(item.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
            font->Render(item.c_str(),
                         -1,
                         FTPoint(width / 2.0f - box.Upper().X() / 2.0f,
                         start - i * (padding + font->Ascender())));
        }
    }
}