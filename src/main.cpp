#include <fstream>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <glad/gl.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/version.h>     // __EMSCRIPTEN_major__ etc.
#include <emscripten/emscripten_mainloop_stub.h>
#endif

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <argparse/argparse.hpp>

#include "tr/tr_texture.h"
#include "tr/tr_window.h"
#include "tr/tr_shader.h"
#include "tr/tr_framebuffer.h"
#include "tr/resource.h"

void CheckGLError(const char* function) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in " << function << ": " << err << std::endl;
    }
}

GLuint VAO, VBO, EBO;

void test_init()
{
    float vertices[] = {
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,

        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        // // first triangle
        // -0.9f, -0.5f, 0.0f,  // left 
        // -0.0f, -0.5f, 0.0f,  // right
        // -0.45f, 0.5f, 0.0f,  // top 
        // // second triangle
        //  0.0f, -0.5f, 0.0f,  // left
        //  0.9f, -0.5f, 0.0f,  // right
        //  0.45f, 0.5f, 0.0f   // top 
    }; 

    unsigned int indices[] = { 
        0, 1, 3,
        1, 2, 3,
     };

    //GLuint VAO;
    glGenVertexArrays(1, &VAO); // No crash here now!
    
    //GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void test(tr::framebuffer& fbo)
{
    tr::scope buffer(fbo);
    // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glBindVertexArray(VAO); 
    // set the count to 6 since we're drawing 6 vertices now (2 triangles); not 3!
    glDrawArrays(GL_TRIANGLES, 0, 6); 
}

void init_imgui(tr::tr_window& wnd)
{
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

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(wnd.window(), wnd.context());
    ImGui_ImplOpenGL3_Init(wnd.glsl_version().c_str());

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
    // auto font_data = tr::resource::load("fonts/roboto/Roboto-VariableFont_wdth,wght.ttf");
    // ImFont* font = io.Fonts->AddFontFromMemoryTTF(font_data.data(), font_data.size(), 20.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);
}


// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void ShowDockingDisabledMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::SmallButton("click here"))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

// void ShowExampleAppDockSpace(bool* p_open)
// {
//     // Variables to configure the Dockspace example.
//     static bool opt_fullscreen = true; // Is the Dockspace full-screen?
//     static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?
//     static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

//     // In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
//     // We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.
//     //
//     // ImGuiWindowFlags_MenuBar is to show a menu bar with config options. This isn't necessary to the functionality of a
//     // Dockspace, but it is here to provide a way to change the configuration flags interactively.
//     // You can remove the MenuBar flag if you don't want it in your app, but also remember to remove the code which actually
//     // renders the menu bar, found at the end of this function.
//     ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

//     // Is the example in Fullscreen mode?
//     if (opt_fullscreen)
//     {
//         // If so, get the main viewport:
//         const ImGuiViewport* viewport = ImGui::GetMainViewport();

//         // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
//         // completely covers the main viewport, giving it a "full-screen" feel.
//         ImGui::SetNextWindowPos(viewport->WorkPos);
//         ImGui::SetNextWindowSize(viewport->WorkSize);
//         ImGui::SetNextWindowViewport(viewport->ID);

//         // Set the parent window's styles to match that of the main viewport:
//         ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
//         ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window

//         // Manipulate the window flags to make it inaccessible to the user (no titlebar, resize/move, or navigation)
//         window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//         window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//     }
//     else
//     {
//         // The example is not in Fullscreen mode (the parent window can be dragged around and resized), disable the
//         // ImGuiDockNodeFlags_PassthruCentralNode flag.
//         dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
//     }

//     // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
//     // and handle the pass-thru hole, so the parent window should not have its own background:
//     if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
//         window_flags |= ImGuiWindowFlags_NoBackground;

//     // If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
//     if (!opt_padding)
//         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

//     // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
//     // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
//     // all active windows docked into it will lose their parent and become undocked.
//     // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
//     // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
//     ImGui::Begin("DockSpace Demo", p_open, window_flags);

//     // Remove the padding configuration - we pushed it, now we pop it:
//     if (!opt_padding)
//         ImGui::PopStyleVar();

//     // Pop the two style rules set in Fullscreen mode - the corner rounding and the border size.
//     if (opt_fullscreen)
//         ImGui::PopStyleVar(2);

//     // Check if Docking is enabled:
//     ImGuiIO& io = ImGui::GetIO();
//     if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
//     {
//         // If it is, draw the Dockspace with the DockSpace() function.
//         // The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
//         ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
//         ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
//     }
//     else
//     {
//         // Docking is DISABLED - Show a warning message
//         ShowDockingDisabledMessage();
//     }

//     // This is to show the menu bar that will change the config settings at runtime.
//     // If you copied this demo function into your own code and removed ImGuiWindowFlags_MenuBar at the top of the function,
//     // you should remove the below if-statement as well.
//     if (ImGui::BeginMenuBar())
//     {
//         if (ImGui::BeginMenu("Options"))
//         {
//             // Disabling fullscreen would allow the window to be moved to the front of other windows,
//             // which we can't undo at the moment without finer window depth/z control.
//             ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
//             ImGui::MenuItem("Padding", NULL, &opt_padding);
//             ImGui::Separator();

//             // Display a menu item for each Dockspace flag, clicking on one will toggle its assigned flag.
//             if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
//             if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
//             if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
//             if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
//             if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
//             ImGui::Separator();

//             // Display a menu item to close this example.
//             if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
//                 if (p_open != NULL) // Remove MSVC warning C6011 (NULL dereference) - the `p_open != NULL` in MenuItem() does prevent NULL derefs, but IntelliSense doesn't analyze that deep so we need to add this in ourselves.
//                     *p_open = false; // Changing this variable to false will close the parent window, therefore closing the Dockspace as well.
//             ImGui::EndMenu();
//         }

//         // Show a help marker that gives an overview of what this example is and does.
//         HelpMarker(
//             "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
//             "- Drag from window title bar or their tab to dock/undock." "\n"
//             "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
//             "- Hold SHIFT to disable docking." "\n"
//             "This demo app has nothing to do with it!" "\n\n"
//             "This demo app only demonstrates the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
//             "Read comments in ShowExampleAppDockSpace() for more details.");

//         ImGui::EndMenuBar();
//     }

//     // End the parent window that contains the Dockspace:
//     ImGui::End();
// }

void ShowExampleAppDockSpace(bool* p_open, bool resize)
{
    // READ THIS !!!
    // TL;DR; this demo is more complicated than what most users you would normally use.
    // If we remove all options we are showcasing, this demo would become:
    //     void ShowExampleAppDockSpace()
    //     {
    //         ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    //     }
    // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
    // In this specific demo, we are not using DockSpaceOverViewport() because:
    // - (1) we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
    // - (2) we allow the host window to have padding (when opt_padding == true)
    // - (3) we expose many flags and need a way to have them visible.
    // - (4) we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport()
    //      in your code, but we don't here because we allow the window to be floating)

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if(opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
        window_flags |= ImGuiWindowFlags_NoBackground;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if(!opt_padding) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    ImGui::Begin("DockSpace Demo", p_open, window_flags);
    if(!opt_padding) {
        ImGui::PopStyleVar();
    }

    if(opt_fullscreen) {
        ImGui::PopStyleVar(2);
    }

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    } else {
        ShowDockingDisabledMessage();
    }

    if(ImGui::BeginMenuBar()) {
        if(ImGui::BeginMenu("Options")) {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if(ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
            if(ImGui::MenuItem("Flag: NoDockingSplit",         "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
            if(ImGui::MenuItem("Flag: NoUndocking",            "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
            if(ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                   { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if(ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if(ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
                *p_open = false;
            ImGui::EndMenu();
        }
        HelpMarker(
            "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
            "- Drag from window title bar or their tab to dock/undock." "\n"
            "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
            "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
            "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
            "This demo app has nothing to do with enabling docking!" "\n\n"
            "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
            "Read comments in ShowExampleAppDockSpace() for more details.");

        ImGui::EndMenuBar();
    }

    static bool is_init = true;
    if(is_init || resize) {
        is_init = false;
        ImGuiID parent_node = ImGui::DockBuilderAddNode();
        ImGui::DockBuilderSetNodePos(parent_node, ImGui::GetWindowPos());
        ImGui::DockBuilderSetNodeSize(parent_node, ImGui::GetWindowSize());

        ImGuiID settings_node = ImGui::DockBuilderAddNode();
        ImGuiID main_node = ImGui::DockBuilderAddNode();

        ImGuiID node_test;
        ImGuiID node_settings;
        ImGuiID node_game;
        ImGuiID node_controls;
        
        ImGui::DockBuilderSplitNode(parent_node, ImGuiDir_Left, 0.2f, &settings_node, &main_node);
        ImGui::DockBuilderSplitNode(settings_node, ImGuiDir_Up, 0.5f, &node_settings, &node_test);
        ImGui::DockBuilderSplitNode(main_node, ImGuiDir_Down, 0.2f, &node_controls, &node_game);

        ImGui::DockBuilderDockWindow("Settings", node_settings);
        ImGui::DockBuilderDockWindow("Test", node_test);
        ImGui::DockBuilderDockWindow("Game", node_game);
        ImGui::DockBuilderDockWindow("Controls", node_controls);

        ImGui::DockBuilderFinish(parent_node);
    }

    ImGui::End();
}

void dump_gl_extensions(int verbosity)
{
    GLint extension_cnt = 0; 
    glGetIntegerv(GL_NUM_EXTENSIONS, &extension_cnt);
    spdlog::info("Number of OpenGL extensions: {}", extension_cnt);

    if(verbosity > 3) {
        // Dump all OpenGL extensions
        for(auto n = 0; n != extension_cnt; ++n) {
            const GLubyte* ext = glGetStringi(GL_EXTENSIONS, n);
            if(ext) {
                spdlog::info("OpenGL extension {}: {}", n, (const char*) ext);
            } else {
                spdlog::warn("Failed to get OpenGL extension {}: {}", n, glGetError() );
            }
        }
    }
}   

#include <filesystem>

int main(int argc, char* argv[])
{
    auto console = spdlog::stdout_color_mt("console");

    bool windowed{ false };
    size_t width{ 1440 };
    size_t height{ 1024 };
    int verbosity{ 0 };
    std::string resource_path;

    argparse::ArgumentParser program(argv[0], "1.0", argparse::default_arguments::none);
    program.add_argument("--help")
        .action([&](const std::string& s) { std::cout << program << std::endl; })
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("shows help message");
    program.add_argument("-V", "--verbose").action([&](const auto&) { ++verbosity; }).append().default_value(false).implicit_value(true).nargs(0);
    program.add_argument("-w", "--width").default_value(width).nargs(1).scan<'d', size_t>().store_into(width);
    program.add_argument("-h", "--height").default_value(height).nargs(1).scan<'d', size_t>().store_into(height);
    program.add_argument("--windowed").default_value(windowed).nargs(0).implicit_value(true).store_into(windowed);
    program.add_argument("-r", "--resources", "--resource-path").default_value("resources").nargs(1).store_into(resource_path);
    // program.add_argument("--font-size").default_value(font_size).store_into(font_size);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        spdlog::critical("Parsing command line arguments failed");
        // Print help for arguments.
        std::cout << program;
        std::exit(1);
    }

    if(verbosity > 2) {
        spdlog::set_level(spdlog::level::debug);
    } else if(verbosity > 1) {
        spdlog::set_level(spdlog::level::info);
    } else if(verbosity > 0) {
        spdlog::set_level(spdlog::level::warn);
    } else {
        spdlog::set_level(spdlog::level::err);
    }

    tr::tr_window main_window{ "SDL3 Tutorial: Hello SDL3+OpenGL3", width, height, !windowed };

    if(!main_window.init()) {
        std::exit(1);
    }

    if(!main_window.create()) {
        std::exit(1);
    }

    dump_gl_extensions(verbosity);

    // XXX Load resources here
    tr::resource::set_resource_path(resource_path);
    nlohmann::json game_data = tr::resource::load_json("game.cfg");
    tr::tr_shader_list shaders;
    for(auto& resource : game_data.items()) {
        if(resource.key() == "shader_programs") {
            shaders = tr::load_shaders(resource.value());
        } else {
            /// XXX
        }
    }

    init_imgui(main_window);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool show_demo_window{ true };
    bool show_another_window{ false };
    bool resize{ true };

    test_init();
    tr::framebuffer fbo(width, height);

    // The running flag
    bool running{ true };

    ImGuiIO& io = ImGui::GetIO();
    auto font_data = tr::resource::load_binary("fonts/roboto/Roboto-VariableFont_wdth,wght.ttf");
    ImFontConfig font_cfg = ImFontConfig();
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(font_data.data(), font_data.size(), 20.0f, &font_cfg);

    //The event data
    SDL_Event e;
    SDL_zero(e);
    //The main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN {
#else // !__EMSCRIPTEN__
    while(running) {
#endif // __EMSCRIPTEN__
        //Get event data
        while(SDL_PollEvent(&e)) {
            
            ImGui_ImplSDL3_ProcessEvent(&e);

            //If event is quit type
            if(e.type == SDL_EVENT_QUIT) {
                //End the main loop
                running = false;
            } else if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(main_window.window())) {
                running = false;
            } else if(e.type == SDL_EVENT_WINDOW_RESIZED) {
                width = e.window.data1;
                height = e.window.data2;
                resize = true;
            } else if(e.type == SDL_EVENT_WINDOW_MAXIMIZED) {
                // width and height don't come from e.window.data1/data2
                int w, h;
                SDL_GetWindowSize(main_window.window(), &w, &h);
                width = static_cast<size_t>(w);
                height = static_cast<size_t>(h);
                resize = true;
            }
        }

        if(SDL_GetWindowFlags(main_window.window()) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ShowExampleAppDockSpace(&show_demo_window, resize);

        ImGui::Begin("Settings");
        ImGui::Text("Hello World a");
        ImGui::Text("Hello World a");
        ImGui::Text("Hello World a");
        ImGui::Text("Hello World a");
        ImGui::Text("Hello World a");
        ImGui::Text("Hello World a");
        ImGui::Text("Hello World a");
        ImGui::End();

        ImGui::Begin("Test");
        ImGui::Text("Hello World b");
        ImGui::End();

        ImGui::Begin("Controls");
        ImGui::Text("Hello World c");
        ImGui::End();

        // if(show_demo_window) {
        //     ShowExampleAppDockSpace(&show_demo_window);
        // }
        
        // if(show_demo_window) {
        //     ImGui::ShowDemoWindow(&show_demo_window);
        // }

        // if(true) {
        //     static float f = 0.0f;
        //     static int counter = 0;

        //     ImGui::Begin("Properties");                          // Create a window called "Hello, world!" and append into it.

        //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //     ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        //     ImGui::Checkbox("Another Window", &show_another_window);

        //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //     ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        //     if(ImGui::Button("Button")) {                          // Buttons return true when clicked (most widgets return true when edited/activated)
        //         counter++;
        //     }
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);

        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        //     ImGui::End();
        // }

        // if(show_another_window) {
        //     ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        //     ImGui::Text("Hello from another window!");
        //     if(ImGui::Button("Close Me")) {
        //         show_another_window = false;
        //     }
        //     ImGui::End();
        // }

        shaders.front().apply();
        // Renders the code to a texture attached to the FBO
        test(fbo);
        ImGui::Begin("Game");
        if(resize) {
            ImVec2 v = ImGui::GetContentRegionAvail();
            spdlog::info("Resize content size {} x {}", v.x, v.y);
            fbo.resize(v.x, v.y);
        }
        // // Render the FBO texture to an imgui window.
        ImGui::Image(fbo.texture_id(), ImVec2(fbo.width(), fbo.height()), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();


        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        // Update the surface
        main_window.swap();

        resize = false;
    } // while(running)
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif // __EMSCRIPTEN__

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //Clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    
    main_window.destroy();
    main_window.quit();

    return 0;
}
