#pragma once

#include <GLUT/GLUT.h>

#include <iostream>
#include <stdlib.h>
#include <string>
#include "Bitmap.h"

class Texture
{
public:
    Texture(GLenum format);
    Texture(GLfloat width, GLfloat height, GLenum format);
    Texture(GLfloat width, GLfloat height, GLenum format, GLfloat data[]);
    Texture(std::string filename);
    ~Texture();
    
    /** Loads this texture onto the GPU */
    virtual void Bind();
    
    /** Returns the texture's id */
    GLuint GetID();
    
protected:
    GLfloat width, height;
    GLfloat *data;
    GLuint id;
    GLenum format;
    
    Bitmap *bitmap;
    
    void LoadTexFile(char *filename);
};
