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
    void SetProjection(float win_w, float win_h);
    void SetPosition(glm::vec3 p) { position = p; }

protected:
	ModelBuffer *mb;
    glm::mat4 projection;
    glm::vec3 position;
};

class CDIndicator : public HUDElement
{
public:
    CDIndicator(int w, int h, string text);
    ~CDIndicator();
    
    virtual void Draw(const Program &p, Scene* s) const;

private:
    ArrayBuffer<glm::vec3> *ab;
    string text;
    
    Program *plain;
};

class Minimap : public HUDElement
{
public:
	Minimap(int w, int h, Texture *tex, Level *l);
    ~Minimap();
    
	virtual void Draw(const Program &p, Scene* s) const;
    
protected:
	Texture *texture;
	ModelBuffer *outlineMB;
	ModelBuffer *pathMB;
	ModelBuffer *shipMB;
    glm::mat4 shipModel;
    
    Program *plain;
};

class Reticle : public HUDElement
{
public:
    Reticle(int w, int h, Texture *tex);
    ~Reticle();
    
    virtual void Draw(const Program &p, Scene* s) const;
    
protected:
    Texture *texture;
};
