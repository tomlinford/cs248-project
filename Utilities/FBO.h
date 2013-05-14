#pragma once

#include "gl.h"

#include <iostream>
#include "Texture.h"
#include "RenderBuffer.h"

class FBO
{
public:
    FBO(GLfloat width, GLfloat height);
    ~FBO();
    
    /** Use, unuse */
    void Use();
    void Unuse();
    void SetDrawTarget(GLenum target);
    
    /** Set associated textures */
    void SetColorTexture(Texture *texture, GLenum target);
    void SetDepthTexture(Texture *texture);
    void SetRenderBuffer(RenderBuffer *renderBuffer);
    
private:
    GLuint id;
    
    void CheckStatus();
};
