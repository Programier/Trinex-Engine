#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/colors.hpp>
#include <Core/logger.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader_parameters.hpp>
#include <Window/window.hpp>

namespace Engine
{
    DefaultClient::DefaultClient()
    {
    }

    DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
    {
        viewport->window()->imgui_initialize();
        return *this;
    }

    DefaultClient& DefaultClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();
        viewport->rhi_clear_color(Color(0, 0, 0, 1));
        viewport->window()->imgui_window()->rhi_render();

        return *this;
    }

    DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
    {
        auto window = viewport->window()->imgui_window();
        window->new_frame();
        ImGui::Begin("Hello World");
        static Vector4D test;
        ImGui::ColorEdit4("Test", &test.x);
        ImGui::End();
        window->end_frame();
        return *this;
    }

    implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
