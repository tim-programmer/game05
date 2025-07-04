#pragma once

#include "tr_scope.h"

namespace tr {

// Represents a single instance of a frame buffer.
class framebuffer : public scoped_object
{
public:
    explicit framebuffer(size_t width, size_t height);    
    virtual ~framebuffer() override;
    void resize(size_t width, size_t height);
    void apply() override;
    void unapply() override;
    unsigned width() const { return width_; }
    unsigned height() const { return height_; }
    unsigned texture_id() const { return tex_; }
private:
    /// @brief Helper function to unbind the currently bound buffers.
    void unbind();
    /// @brief Width of the frame buffer.
    size_t width_{ 0 };
    /// @brief Height of the frame buffer.
    size_t height_{ 0 };
    /// @brief  Frame buffer object.
    unsigned fbo_{ 0 };
    /// @brief  Render buffer object.
    unsigned rbo_{ 0 };
    /// @brief ID for the texture bound to the framebuffer.
    unsigned tex_{ 0 };
};

typedef std::unique_ptr<framebuffer> framebuffer_ptr_t;

}