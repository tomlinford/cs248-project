#include "Texture.h"

using namespace::std;

Texture::Texture(GLenum format)
{
    Texture(0, 0, format);
}

Texture::Texture(GLfloat width, GLfloat height, GLenum format)
{
    Texture(0, 0, format, NULL);
}

Texture::Texture(GLfloat width, GLfloat height, GLenum format, GLfloat data[])
{
    Texture::width = width;
    Texture::height = height;
    Texture::format = format;
    glGenTextures(1, &id);
    Texture::data = data;
    bitmap = NULL;
    Bind();
}

Texture::Texture(string filename)
{
    bitmap = new Bitmap();
    LoadTexFile((char *)filename.c_str());
    width = bitmap->width;
    height = bitmap->height;
    Bind();
}

Texture::~Texture()
{
    glDeleteTextures(1, &id);
    if (bitmap)
        delete bitmap;
}

/* Loads a texture file into the shader */
void Texture::LoadTexFile(char *filename)
{
    if (!bitmap->loadBMP(filename))
        cout << "Texture " << filename << " failed to load" << endl;
}

GLuint Texture::GetID()
{
    return id;
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, id);
    if (bitmap) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, bitmap->data);
    }
    else if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_FLOAT, data);
    }
    else {
        if (format == GL_DEPTH_COMPONENT)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei) width, (GLsizei) height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei) width, (GLsizei) height, 0, format, GL_UNSIGNED_BYTE, NULL);
    }
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}
