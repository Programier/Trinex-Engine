#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Graphics/imgui.hpp>
#include <ImGui/imgui.h>
#include <Window/window.hpp>
#include <api.hpp>
#include <imgui_impl_sdl.h>


namespace Engine::ImGuiRenderer
{

    static struct ImGuiData {
        ImGuiContext* context = nullptr;
        SDL_Window* window    = nullptr;
    } imgui_data;


    static void process_event(void* sdl_event)
    {
        ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(sdl_event));
    }

    ENGINE_EXPORT void init()
    {
        if (imgui_data.context)
        {
            return;
        }

        if (Window::instance() == nullptr)
        {
            logger->error("ImGuiRenderer: Cannot init ImGUI. Create window first!");
            return;
        }

        IMGUI_CHECKVERSION();
        imgui_data.context = ImGui::CreateContext();


        imgui_data.window = static_cast<SDL_Window*>(Window::instance()->SDL());
        switch (engine_instance->api())
        {
            case Engine::EngineAPI::OpenGL:
                ImGui_ImplSDL2_InitForOpenGL(imgui_data.window, Window::instance()->SDL_OpenGL_context());
                break;

            case Engine::EngineAPI::Vulkan:
                ImGui_ImplSDL2_InitForVulkan(imgui_data.window);
                break;

            default:
                break;
        }

        engine_instance->api_interface()->imgui_init();
        Event::sdl_callbacks.insert(process_event);

        DestroyController controller(ImGuiRenderer::terminate);
    }

    ENGINE_EXPORT void terminate()
    {
        Event::sdl_callbacks.erase(process_event);
        engine_instance->api_interface()->imgui_terminate();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(imgui_data.context);

        new (&imgui_data) ImGuiData();
    }

    ENGINE_EXPORT void render()
    {
        ImGui::Render();
        engine_instance->api_interface()->imgui_render();
    }

    ENGINE_EXPORT void new_frame()
    {
        engine_instance->api_interface()->imgui_new_frame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }
}// namespace Engine::ImGuiRenderer
