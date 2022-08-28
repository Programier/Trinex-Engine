set(SOURCES imgui.cpp
    imgui_draw.cpp
    imgui_impl_sdl.cpp
    imgui_impl_sdlrenderer.cpp
    imgui_impl_opengl2.cpp
    imgui_impl_opengl3.cpp
    imgui_impl_vulkan.cpp
    imgui_tables.cpp
    imgui_filebrowser.cpp
    imgui_widgets.cpp)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
