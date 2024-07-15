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
    static Name screen_texture_name = "screen_texture";

    DefaultClient::DefaultClient()
    {
        m_vertex_buffer         = Object::new_instance<TypedVertexBuffer<Vertex>>();
        m_vertex_buffer->buffer = {{Color(0, 0, 0, 1.f), {0.0, 0.0}},    //
                                   {Color(0, 1.0, 0, 1.f), {0.f, 1.f}},  //
                                   {Color(1.0, 1.f, 0, 1.f), {1.f, 1.f}},//
                                   {Color(1.f, 0, 0, 1.f), {1.f, 0.f}}}; //
        m_vertex_buffer->init_resource();

        m_index_buffer         = Object::new_instance<UInt32IndexBuffer>();
        m_index_buffer->buffer = {0, 1, 2, 0, 2, 3};
        m_index_buffer->init_resource();

        m_material = Object::load_object("Test::Test")->instance_cast<Material>();
        info_log("DefaultClient", "Binding index of globals is '%d'", int(m_material->pipeline->global_parameters.bind_index()));
    }

    DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
    {
        //viewport->window()->imgui_initialize();
        return *this;
    }

    DefaultClient& DefaultClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();

        // m_material->apply();
        // m_vertex_buffer->rhi_bind(0);
        // m_index_buffer->rhi_bind(0);
        // rhi->draw_indexed(6, 0, 0);

        // /viewport->window()->imgui_window()->rhi_render();

        return *this;
    }

    DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
    {
        // auto window = viewport->window()->imgui_window();
        // window->new_frame();
        // ImGui::Begin("Hello World");
        // static Vector4D test;
        // ImGui::ColorEdit4("Test", &test.x);
        // ImGui::End();
        // window->end_frame();
        return *this;
    }

    implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
