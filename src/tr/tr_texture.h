#pragma once

#include <string_view>
#include <SDL3/SDL.h>

namespace tr
{

class tr_texture
{
public:
    tr_texture();
    virtual ~tr_texture();

    bool load_from_file(SDL_Renderer* renderer, const std::string_view& filename);
    void destroy();
    void render(SDL_Renderer* renderer, float x, float y);
    size_t width() const { return width_; }
    size_t height() const { return height_; }
private:
    size_t width_{ 0 };
    size_t height_{ 0 };
    SDL_Texture* texture_{ nullptr };

    tr_texture(const tr_texture&) = delete;
    tr_texture(tr_texture&&) = delete;
    tr_texture& operator=(const tr_texture&) = delete;
    tr_texture& operator=(tr_texture&&) = delete;
};

}
