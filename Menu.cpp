#include "Menu.h"

using namespace::std;

Menu::Menu(std::string o[], menuFunc f[], int i)
{
    options = o;
    functions = f;
    numItems = i;
    
    next = NULL;
    previous = NULL;
    
    selectedIndex = 0;
    
    padding = 20.0f;
    
    font = new FTGLPixmapFont("01 Digitall.ttf");
	font->FaceSize(72);
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
    if (previous)
        previous->SetNext(NULL);
}

void Menu::HandleKey(int key, int action)
{
    if (next) {
        next->HandleKey(key, action);
        return;
    }
    
    if (action == GLFW_RELEASE)
        return;
    
    switch(key) {
        case GLFW_KEY_UP:
            selectedIndex--;
            break;
        case GLFW_KEY_DOWN:
            selectedIndex++;
            break;
        case GLFW_KEY_ENTER:
        {
            if (functions[selectedIndex])
                functions[selectedIndex](NULL);
            break;
        }
        case GLFW_KEY_ESC:
            PopMenu();
            break;
        default:
            break;
    }
    
    if (selectedIndex < 0)
        selectedIndex = numItems - 1;
    else if (selectedIndex == numItems)
        selectedIndex = 0;
    cout << "Selected index " << selectedIndex << endl;
}

void Menu::Render()
{
    if (next) {
        next->Render();
        return;
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    float itemHeight = font->Ascender() * numItems + padding * (numItems - 1);
    float start = height / 2.0f + itemHeight / 2.0f;
    
    for (int i = 0; i < numItems; i++) {
        
        if (i == selectedIndex) {
            glColor4f(0.0, 0.7, 0.9, 1.0);
            //continue;
        }
        else {
            glColor4f(0.5, 0.5, 0.5, 1.0);
        }
        glWindowPos2f(0,0);
        
        string item = options[i];
        FTBBox box = font->BBox(item.c_str(), -1, FTPoint(0, 0), FTPoint(0, 0));
        font->Render(item.c_str(),
                     -1,
                     FTPoint(width / 2.0f - box.Upper().X() / 2.0f,
                     start - i * (padding + font->Ascender())));
    }
}