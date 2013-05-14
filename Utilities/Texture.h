#pragma once

#include "gl.h"

#include <stdlib.h>

class Texture
{
public:
    Texture(GLenum format);
    Texture(GLfloat width, GLfloat height, GLenum format);
    ~Texture();
    
    /** Loads this texture onto the GPU */
    virtual void Bind();
    
    /** Returns the texture's id */
    GLuint GetID();
    
protected:
    GLfloat width, height;
    GLuint id;
    GLenum format;
};
