cmake_minimum_required(VERSION 3.20)

project(game05)

enable_language(CXX C ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

set(BUILD_SHARED_LIBS FALSE)

find_package(OpenGL REQUIRED)

set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

FetchContent_Declare(spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog.git
    GIT_TAG         v1.15.3
)
FetchContent_MakeAvailable(spdlog)

set(SDL_STATIC ON CACHE INTERNAL "")
set(SDL_SHARED OFF CACHE INTERNAL "")

FetchContent_Declare(
    SDL3 
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG         release-3.2.12
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)

FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.2.4
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SDL3 SDL3_ttf SDL3_image)

FetchContent_Declare(argpase
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG        v3.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(argpase)

FetchContent_Declare(gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(gtest)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        release-1-0-2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v6.0.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9b-docking
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG        v4.1.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(flecs)

FetchContent_Declare(rapidyaml
    GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
    GIT_TAG        v0.9.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(rapidyaml)


add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external/include)

add_library(imgui-lib)
target_sources(imgui-lib PUBLIC 
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui-lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui-lib PUBLIC SDL3::SDL3)

add_library(tr STATIC
    src/tr/tr_window.cpp
    src/tr/tr_texture.cpp
    src/tr/tr_shader.cpp
    src/tr/tr_scope.cpp
    src/tr/tr_framebuffer.cpp
    src/tr/resource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/external/src/gl.c
    #${CMAKE_CURRENT_LIST_DIR}/external/src/gles2.c
)
target_include_directories(tr INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/tr)
target_include_directories(tr PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external/include)
target_link_libraries(tr SDL3::SDL3-static SDL3_image::SDL3_image-static spdlog OpenGL::GL)


target_link_libraries(${PROJECT_NAME} 
    tr
    argparse
    gtest
    spdlog   
    imgui-lib
    glm
    flecs
    SDL3::SDL3-static
)
