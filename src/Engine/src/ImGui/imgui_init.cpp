#include <ImGui-Private/imgui_impl_opengl3.h>
#include <ImGui-Private/imgui_impl_sdl.h>
#include <Init/init.hpp>
#include <Window/window.hpp>
#include <imgui_init.hpp>

namespace Engine::ImGuiInit
{

    static void opengl_imgui_init(const char* glsl_version)
    {
        ImGui_ImplSDL2_InitForOpenGL((SDL_Window*) Window::SDL(), Window::SDL_OpenGL_context());
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    static void vulkan_imgui_init(const char* glsl_version)
    {
        throw not_implemented;
    }

    static void (*init_imgui[2])(const char*) = {opengl_imgui_init, vulkan_imgui_init};


    static void opengl_imgui_terminate()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
    }

    static void vulkan_imgui_terminate()
    {
        throw not_implemented;
    }

    static void (*terminate_imgui[2])() = {opengl_imgui_terminate, vulkan_imgui_terminate};


    static void opengl_imgui_render()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    static void vulkan_imgui_render()
    {
        throw not_implemented;
    }

    static void (*render_imgui[2])() = {opengl_imgui_render, vulkan_imgui_render};

    static void opengl_imgui_frame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
    }

    static void vulkan_imgui_frame()
    {
        throw not_implemented;
    }

    static void (*frame_imgui[2])() = {opengl_imgui_frame, vulkan_imgui_frame};


    static void imgui_event(void* event)
    {
        ImGui_ImplSDL2_ProcessEvent(static_cast<const SDL_Event*>(event));
    }

    void init(const char* glsl_version)
    {
        init_imgui[cast(int, Engine::API())](glsl_version);
        Window::event_callbacks().push_back(imgui_event);
    }

    void terminate()
    {
        terminate_imgui[cast(int, Engine::API())]();
    }

    void render()
    {
        render_imgui[cast(int, Engine::API())]();
    }

    void new_frame()
    {
        frame_imgui[cast(int, Engine::API())]();
    }

}// namespace Engine::ImGuiInit