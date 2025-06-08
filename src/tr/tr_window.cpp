#include <spdlog/spdlog.h>
#include "tr_window.h"

namespace tr {

    tr_window::tr_window()
    {
    }

    tr_window::tr_window(std::string_view caption, size_t screen_width, size_t screen_height)
        : caption_(caption)
        , screen_width_(screen_width)
        , screen_height_(screen_height)
    {
    }

    tr_window::~tr_window()
    {
        destroy();
    }


    bool tr_window::create()
    {
        const SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY; 

        if(SDL_CreateWindowAndRenderer(caption_.c_str(), screen_width_, screen_height_, window_flags, &window_, &renderer_)) {            
            SDL_SetRenderVSync(renderer_, 1);
            SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(window_);
            return true;
        } else {
            spdlog::critical("Window could not be created! SDL error: {}\n", SDL_GetError());
        }

        return false;
    }

    void tr_window::destroy()
    {
        if(window_ != nullptr) {
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
    }

    bool tr_window::init()
    {
        if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            spdlog::critical("SDL could not initialize! SDL error: {}\n", SDL_GetError());
            return false;
        }
        return true;
    }

    void tr_window::quit()
    {
        SDL_Quit();
    }

    std::unique_ptr<tr_texture> tr_window::create_texture_from_file(std::string_view filename)
    {
        auto tex = std::make_unique<tr_texture>();
        if(tex->load_from_file(renderer_, filename)) {
            return tex;
        }
        return nullptr;
    }
}
