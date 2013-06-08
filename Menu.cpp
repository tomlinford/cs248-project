#include "Menu.h"

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
    
    selectedIndex = 0;
    selectionActive = false;
    
    padding = 20.0f;
    
    font = new FTGLPixmapFont("01 Digitall.ttf");
	font->FaceSize(fontsize);
    
    timer = new cpu_timer();
}

Menu::~Menu()
{
    delete font;
}

void Menu::PushMenu(Menu *other)
{
    next = other;
    other->SetPrevious(this);
}

void Menu::PopMenu()
{
    if (previous) {
        previous->SetNext(NULL);
        previous->SetSelectionActive(false);
    }
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
    if (selectionActive) {
        items[selectedIndex]->HandleKey(key, action);
    }
    
    switch(key) {
        case GLFW_KEY_UP:
            if (selectionActive)
                selectionActive = false;
            UpdateSelection(-1);
            break;
        case GLFW_KEY_DOWN:
            if (selectionActive)
                selectionActive = false;
            UpdateSelection(1);
            break;
        case GLFW_KEY_ENTER:
        {
            selectionActive = true;
            if (items[selectedIndex]->func)
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

void Menu::Render()
{
	//cout << "Back to rendering" << endl;
    if (next) {
        next->Render();
		//cout << "Submenu rendering" << endl;
        return;
    }
	//cout << "My menu rendering" << endl;
    
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
            glColor4f(0.5, 0.5, 0.5, 1.0);
        }
        glWindowPos2f(0,0);
        
        string item = items[i]->label;
        FTBBox box = font->BBox(item.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
        font->Render(item.c_str(),
                     -1,
                     FTPoint(width / 2.0f - box.Upper().X() / 2.0f,
                     start - i * (padding + font->Ascender())));
    }
}