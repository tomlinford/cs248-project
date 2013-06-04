#include "FBO.h"

using namespace std;

FBO::FBO(GLfloat width, GLfloat height)
{
    glGenFramebuffers(1, &id);
}

FBO::~FBO()
{
    glDeleteFramebuffers(1, &id);
}

void FBO::Use()
{
    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    
    // Check status
    // CheckStatus();
    
    // Default: set draw buffer to Color Attachment 0
    GLenum target = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &target);
}

void FBO::SetDrawTarget(GLenum target) {
    glDrawBuffers(1, &target);
}

void FBO::Unuse()
{
    // Unbind FBO (pass a 0 id)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::SetColorTexture(Texture *texture, GLenum target)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, texture->GetID(), 0);
}

void FBO::SetDepthTexture(Texture *texture)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->GetID(), 0);
}

void FBO::SetRenderBuffer(RenderBuffer *renderBuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer->GetID());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::CheckStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
            
        case GL_FRAMEBUFFER_UNSUPPORTED:
            /* Choose different formats */
            cerr << "Framebuffer unsupported\n" << endl;
            break;
            
        default:
            /* Programming error; will fail on all hardware */
            cerr << "Framebuffer error\n" << endl;
            cerr << "Did you attach a texture?" << endl;
            exit(-1);
    }
}