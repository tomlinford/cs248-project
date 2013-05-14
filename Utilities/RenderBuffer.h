#pragma once

#include "gl.h"

class RenderBuffer
{
public:
    RenderBuffer();
    RenderBuffer(float width, float height);
    ~RenderBuffer();
    
    GLuint GetID();
    
private:
    GLuint id;
};
