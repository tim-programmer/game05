#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include "tr_framebuffer.h"

namespace tr {

framebuffer::framebuffer(size_t width, size_t height)
    : width_(width)
    , height_(height)
{
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_, 0);

    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::critical("Framebuffer was not complete.");
        std::exit(1);
    }

    unbind();
}

framebuffer::~framebuffer()
{

}

void framebuffer::unbind()
{
    // These unbind the current frame buffer, render buffer and texture.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void framebuffer::apply()
{
    if(fbo_ == 0) {
        spdlog::critical("Trying to bind an invalid framebuffer object.");
        std::exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void framebuffer::unapply()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::resize(size_t width, size_t height)
{
    width_ = width;
    height_ = height;

    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

}
