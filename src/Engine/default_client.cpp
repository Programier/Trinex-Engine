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
                                   {Color(0, 0, 0, 1.f), {0.0, 0.0}},    //
                                   {Color(1.f, 1.f, 0, 1.f), {1.f, 1.f}},//
                                   {Color(1.f, 0, 0, 1.f), {1.f, 0.f}}}; //
        m_vertex_buffer->init_resource();

        m_material = Object::load_object("Test::Test")->instance_cast<Material>();
        info_log("DefaultClient", "Binding index of globals is '%d'", int(m_material->pipeline->global_parameters.bind_index()));
    }

    DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
    {
        return *this;
    }

    DefaultClient& DefaultClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();

        m_material->apply();
        m_vertex_buffer->rhi_bind(0);
        rhi->draw(6, 0);

        return *this;
    }

    DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
    {
        return *this;
    }

    implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
