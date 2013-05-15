#include "Texture.h"

Texture::Texture(GLenum format)
{
    Texture(0, 0, format);
}

Texture::Texture(GLfloat width, GLfloat height, GLenum format)
{
    Texture::width = width;
    Texture::height = height;
    glGenTextures(1, &id);
    Bind();
}

Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

GLuint Texture::GetID()
{
    return id;
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, id);
    if (format == GL_DEPTH_COMPONENT)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei) width, (GLsizei) height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}
