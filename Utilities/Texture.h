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
    virtual void Bind() const;
    
    /** Accessors the texture's id */
    GLuint GetID() const { return id; }
    GLuint GetWidth() const { return width; }
    GLuint GetHeight() const { return height; }
    GLenum GetFormat() const { return format; }
    GLfloat *GetData() const { return data; }
    
protected:
    GLuint width, height;
    GLfloat *data;
    GLuint id;
    GLenum format;
    
    Bitmap *bitmap;
    
    void LoadTexFile(char *filename) const;
};
