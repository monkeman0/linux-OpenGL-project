cmake_minimum_required(VERSION 3.10.0)
project(linux_opengl_hope VERSION 0.1.0 LANGUAGES C CXX)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)

add_executable(linux_opengl_hope
    src/main.cpp
    src/glad.c
    src/inputHandler.cpp
    src/stb_image.cpp
    resources/libraries/imgui/imgui_draw.cpp
    resources/libraries/imgui/imgui_tables.cpp
    resources/libraries/imgui/imgui_widgets.cpp
    resources/libraries/imgui/imgui.cpp
    resources/libraries/imgui/backends/imgui_impl_glfw.cpp
    resources/libraries/imgui/backends/imgui_impl_opengl3.cpp
)

include_directories(
    resources/libraries/glm
    resources/libraries/imgui
    resources/libraries/imgui/backends
    resources/headers
    resources/data
    resources/shaders
    resources/fonts
    resources/textures
)

add_subdirectory(resources/libraries/freetype)

target_link_libraries(linux_opengl_hope
    glfw
    OpenGL::GL
    freetype
)

