#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Utilities/Texture.h"
#include "Utilities/Buffer.h"

#include "Scene.h"
#include "Level.h"

#define MINIMAP_SIZE 200

class HUDElement
{
public:
	virtual void Draw(const Program &p, Scene* s) const {}

protected:
	ModelBuffer *mb;
    glm::mat4 projection;
};

class CDIndicator : public HUDElement
{
public:
    CDIndicator(int x, int y, int w, int h, int win_w, int win_h, string text);
    ~CDIndicator();
    
    virtual void Draw(const Program &p, Scene* s) const;

private:
    ArrayBuffer<glm::vec3> *ab;
    string text;
};

class Minimap : public HUDElement
{
public:
	Minimap(int x, int y, int w, int h, int win_w, int win_h, Texture *tex, Level *l);
    ~Minimap();
    
	virtual void Draw(const Program &p, Scene* s) const;
    
protected:
	Texture *texture;
	ModelBuffer *outlineMB;
	ModelBuffer *pathMB;
	ModelBuffer *shipMB;
    glm::mat4 shipModel;
};

class Reticle : public HUDElement
{
public:
    Reticle(int x, int y, int w, int h, int win_w, int win_h, Texture *tex);
    ~Reticle();
    
    virtual void Draw(const Program &p, Scene* s) const;
    
protected:
    Texture *texture;
};
