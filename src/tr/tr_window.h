#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <SDL3/SDL.h>

#include "tr_texture.h"

namespace tr {

class tr_window
{
public:
    tr_window();
    tr_window(std::string_view caption, size_t screen_width, size_t screen_height);
    virtual ~tr_window();
    size_t width() const { return screen_width_; }
    size_t height() const { return screen_height_; } 
    bool init();
    void quit();
    bool create();
    void destroy();
    void set_dimensions(size_t screen_width, size_t screen_height) { screen_width_ = screen_width; screen_height_ = screen_height; }
    void set_caption(std::string_view caption) { caption_ = caption; }
    SDL_Renderer* renderer() const { return renderer_; }
    SDL_Window* window() const { return window_; }
    void set_render_hook(std::function<void(*)(SDL_Renderer*)> fn);

    std::unique_ptr<tr_texture> create_texture_from_file(std::string_view filename);
private:
    std::string caption_;
    size_t screen_width_{ 1024 };
    size_t screen_height_{ 768 };
    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };

    tr_window(const tr_window&) = delete;
    tr_window(tr_window&&) = delete;
    tr_window& operator=(const tr_window&) = delete;
    tr_window& operator=(tr_window&&) = delete;
};

}
