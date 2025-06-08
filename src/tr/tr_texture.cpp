#include <string>
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>

#include "tr_texture.h"

namespace tr
{
    tr_texture::tr_texture()
    {
    }

    tr_texture::~tr_texture()
    {
        destroy();
    }

    bool tr_texture::load_from_file(SDL_Renderer* renderer, const std::string_view& filename)
    {
        destroy();

        //Load surface
        if(SDL_Surface* loadedSurface = IMG_Load(std::string(filename).c_str()); loadedSurface == nullptr) {
            spdlog::error("Unable to load image {}! SDL_image error: {}\n", filename, SDL_GetError());
        } else {
            // Create texture from surface
            texture_ = SDL_CreateTextureFromSurface(renderer, loadedSurface);
            
            if(texture_ != nullptr) {
                // Get image dimensions
                width_ = loadedSurface->w;
                height_ = loadedSurface->h;
            } else {
                spdlog::error("Unable to create texture from loaded pixels! SDL error: {}\n", SDL_GetError());
            }

            // Clean up loaded surface
            SDL_DestroySurface(loadedSurface);
        }

        // Return success if texture loaded
        return texture_ != nullptr;        
    }

    void tr_texture::destroy()
    {
        SDL_DestroyTexture(texture_);
        width_ = 0;
        height_ = 0;
        texture_ = nullptr;
    }

    void tr_texture::render(SDL_Renderer* renderer, float x, float y)
    {        
        if(texture_ == nullptr) {
            spdlog::critical("texture was null");
        } else {
            //Set texture position
            SDL_FRect destination{ x, y, static_cast<float>(width_), static_cast<float>(height_) };

            //Render texture
            SDL_RenderTexture(renderer, texture_, nullptr, &destination);
        }
    }

}
