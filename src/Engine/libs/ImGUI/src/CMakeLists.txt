
set(SOURCES imgui.cpp
    imgui_draw.cpp
    imgui_impl_sdl.cpp
    imgui_impl_sdlrenderer.cpp
    imgui_impl_vulkan.cpp
    imgui_tables.cpp
    imgui_filebrowser.cpp
    imgui_widgets.cpp
 #   imgui_impl_opengl2.cpp
    imgui_impl_opengl3.cpp)



if(ANDROID)
    set(DEFS IMGUI_IMPL_OPENGL_ES3)
endif()


add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ${DEFS})
