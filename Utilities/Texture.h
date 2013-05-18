#pragma once

#include "gl.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include "Bitmap.h"

class Texture
{
public:
    /** Creates a blank 512 x 512 texture */
    Texture(GLenum format);
    
    /** Creates a blank texture of the given width, height, and format */
    Texture(GLuint width, GLuint height, GLenum format);
    
    /** Creates a blank texture of the given width, height, and format
     with the given pixel data. The pixel data must match the given
     format. */
    Texture(GLuint width, GLuint height, GLenum format, GLfloat data[]);
    
    /** Creates a blank texture from a bitmap file */
    Texture(std::string filename);
    
    /** Destructor */
    ~Texture();
    
    /** Loads this texture onto the GPU */
    virtual void Bind();
    
    /** Accessors the texture's id */
    GLuint GetID() { return id; }
    GLuint GetWidth() { return width; }
    GLuint GetHeight() { return height; }
    GLenum GetFormat() { return format; }
    GLfloat *GetData() { return data; }
    
protected:
    GLuint width, height;
    GLfloat *data;
    GLuint id;
    GLenum format;
    
    Bitmap *bitmap;
    
    void LoadTexFile(char *filename);
};
