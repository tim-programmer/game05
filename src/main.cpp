#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "tr/tr_texture.h"
#include "tr/tr_window.h"

constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 480 };

std::unique_ptr<tr::tr_texture> tex;
tr::tr_window main_window;

// Starts up SDL and creates window
bool init();

// Loads media
bool loadMedia();

// Frees media and shuts down SDL
void close();

int main(int argc, char* argv[])
{
    auto console = spdlog::stdout_color_mt("console");

    //Final exit code
    int exitCode{ 0 };

    //Initialize
    if(!init()) {
        spdlog::critical("Unable to initialize program!\n");
        exitCode = 1;
    } else {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Multiple view ports
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Allow docking of imgui windows
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLRenderer(main_window.window(), main_window.renderer());
        ImGui_ImplSDLRenderer3_Init(main_window.renderer());

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != nullptr);

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // main_window.set_render_hook([tex](SDL_Renderer* r) {
        //     tex->render(r, 0, 0);
        // });

        bool show_demo_window = true;
        bool show_another_window = false;        

        //Load media
        if(!loadMedia()) {
            spdlog::error("Unable to load media!\n");
            exitCode = 2;
        } else {
            // The running flag
            bool running{ true };

            //The event data
            SDL_Event e;
            SDL_zero(e);
            //The main loop
            while(running) {
                //Get event data
                while(SDL_PollEvent(&e)) {
                    
                    ImGui_ImplSDL3_ProcessEvent(&e);

                    //If event is quit type
                    if(e.type == SDL_EVENT_QUIT) {
                        //End the main loop
                        running = false;
                    }
                    if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(main_window.window())) {
                        running = false;
                    }
                }

                // Start the Dear ImGui frame
                ImGui_ImplSDLRenderer3_NewFrame();
                ImGui_ImplSDL3_NewFrame();
                ImGui::NewFrame();                
                
                if (show_demo_window)
                {
                    ImGui::ShowDemoWindow(&show_demo_window);
                }

                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);

                    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                    ImGui::End();
                }

                if (show_another_window)
                {
                    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui::Text("Hello from another window!");
                    if (ImGui::Button("Close Me"))
                        show_another_window = false;
                    ImGui::End();
                }                

                // Fill the surface white
                SDL_SetRenderDrawColor(main_window.renderer(), 0xff, 0xff, 0xff, 0xff);
                SDL_RenderClear(main_window.renderer());
                // Render image on screen
                tex->render(main_window.renderer(), 0, 0);

                ImGui::Render();
                SDL_SetRenderScale(main_window.renderer(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
                SDL_SetRenderDrawColorFloat(main_window.renderer(), clear_color.x, clear_color.y, clear_color.z, clear_color.w);
                SDL_RenderClear(main_window.renderer());
                ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), main_window.renderer());
                SDL_RenderPresent(main_window.renderer());

                // Update and Render additional Platform Windows
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    // TODO for OpenGL: restore current GL context.
                }

                // Update the surface
                SDL_RenderPresent(main_window.renderer());
            } 
        }
    }
    
    //Clean up
    close();

    return exitCode;                                                
}

bool init()
{
    main_window.set_caption("SDL3 Tutorial: Hello SDL3");
    if(main_window.init()) {
        if(main_window.create()) {
            return true;
        }
    }
    return false;
}

bool loadMedia()
{
    //Load splash image
    std::string image_path{ "resources/preview.png" };

    tex = main_window.create_texture_from_file(image_path);

    if(tex) {        
        return true;
    } else {
        spdlog::error("Unable to load image {}! SDL Error: {}\n", image_path, SDL_GetError());
    }

    return false;
}

void close()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    
    main_window.destroy();
    main_window.quit();   
}