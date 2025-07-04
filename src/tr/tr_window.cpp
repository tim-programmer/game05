#include <spdlog/spdlog.h>
#include <glad/gl.h>

#include "tr_window.h"

namespace {

std::string set_gl_version_attributes()
{
    // Decide GL+GLSL versions
// #if defined(IMGUI_IMPL_OPENGL_ES2)
//     // GL ES 2.0 + GLSL 100 (WebGL 1.0)
//     const char* glsl_version = "#version 100";
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
// #elif defined(IMGUI_IMPL_OPENGL_ES3)
//     // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
//     const char* glsl_version = "#version 300 es";
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
// #elif defined(__APPLE__)
//     // GL 3.2 Core + GLSL 150
//     const char* glsl_version = "#version 150";
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
// #else
    // GL 3.3 + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
// #endif
    return glsl_version;
}

}

namespace tr {

    tr_window::tr_window()
    {
    }

    tr_window::tr_window(std::string_view caption, size_t screen_width, size_t screen_height, bool fullscreen)
        : caption_(caption)
        , screen_width_(screen_width)
        , screen_height_(screen_height)
        , fullscreen_(fullscreen)
    {
    }

    tr_window::~tr_window()
    {
        destroy();
    }

    bool tr_window::create()
    {
        glsl_version_ = set_gl_version_attributes();

        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        
        SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY; 
        if(fullscreen_) {
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }

        window_ = SDL_CreateWindow(caption_.c_str(), screen_width_, screen_height_, window_flags);
        if(window_ != nullptr) {
            context_ = SDL_GL_CreateContext(window_);
            if(context_ != nullptr) {
                SDL_GL_MakeCurrent(window_, context_);

                // Load GL
                if(int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress) != 0) {
                    spdlog::info("OpenGL version: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
                    spdlog::info("OpenGL vendor: {}", (const char*) glGetString(GL_VENDOR));
                    spdlog::info("OpenGL renderer: {}", (const char*) glGetString(GL_RENDERER));
                    spdlog::info("OpenGL GLSL version: {}", (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
                    
                    SDL_GL_SetSwapInterval(1); // Enable vsync
                    SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                    SDL_ShowWindow(window_);
                    return true;
                }

                spdlog::critical("Failed to load OpenGL functions.");
            }

            spdlog::critical("SDL_GL_CreateContext(): {}", SDL_GetError());
        } else {
            spdlog::critical("Window could not be created! SDL error: {}", SDL_GetError());
        }

        return false;
    }

    void tr_window::destroy()
    {
        if(window_ != nullptr) {
            if(context_ != nullptr) {
                SDL_GL_DestroyContext(context_);
                context_ = nullptr;
            }
        
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        quit();
    }

    bool tr_window::init()
    {
        if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            spdlog::critical("SDL could not initialize! SDL error: {}\n", SDL_GetError());
            return false;
        }
        initialised_ = true;
        return true;
    }

    void tr_window::quit()
    {
        if(initialised_)
        {
            SDL_Quit();
            initialised_ = false;
        }
    }

    void tr_window::swap()
    {
        SDL_GL_SwapWindow(window_);
    }

    std::unique_ptr<tr_texture> tr_window::create_texture_from_file(std::string_view filename)
    {
        //auto tex = std::make_unique<tr_texture>();
        // if(tex->load_from_file(renderer_, filename)) {
        //     return tex;
        // }
        return nullptr;
    }
}
