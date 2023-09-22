#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Window/window.hpp>
#include <Graphics/rhi.hpp>
#include <imgui.h>

namespace Engine::ImGuiRenderer
{
    ENGINE_EXPORT void init()
    {
        Window* window = engine_instance->window();

        if (window == nullptr)
        {
            error_log("ImGuiRenderer", "Cannot init ImGUI. Create window first!");
            return;
        }

        window->initialize_imgui();
        engine_instance->api_interface()->imgui_init();
    }

    ENGINE_EXPORT void terminate()
    {
        engine_instance->api_interface()->imgui_terminate();
        engine_instance->window()->terminate_imgui();
    }

    ENGINE_EXPORT void render()
    {
        ImGui::Render();
        engine_instance->api_interface()->imgui_render();
    }

    ENGINE_EXPORT void new_frame()
    {
        engine_instance->api_interface()->imgui_new_frame();
        engine_instance->window()->new_imgui_frame();
        ImGui::NewFrame();
    }
}// namespace Engine::ImGuiRenderer
