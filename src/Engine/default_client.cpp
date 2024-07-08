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
#include <editor_resources.hpp>

namespace Engine
{
    static Name screen_texture_name = "screen_texture";

    DefaultClient::DefaultClient()
    {
        m_vertex_buffer         = Object::new_instance<TexCoordVertexBuffer>();
        m_vertex_buffer->buffer = {{-1.0, -1.0}, {-1.f, 1.f}, {1.f, 1.f}, {-1.0, -1.0}, {1.f, 1.f}, {1.f, -1.f}};
        m_vertex_buffer->init_resource();

        m_material = Object::load_object("Test::Triangle")->instance_cast<Material>();
        info_log("DefaultClient", "Binding index of globals is '%d'", int(m_material->pipeline->global_parameters.bind_index()));
    }

    DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
    {
        return *this;
    }

    DefaultClient& DefaultClient::render(class RenderViewport* viewport)
    {
        RenderSurface* base_rt[1] = {SceneRenderTargets::instance()->surface_of(SceneRenderTargets::Surface::BaseColor)};
        auto param                = m_material->find_parameter(screen_texture_name);

        rhi->bind_render_target(base_rt, nullptr);
        if (param)
        {
            reinterpret_cast<CombinedImageSampler2DMaterialParameter*>(param)->texture = EditorResources::light_sprite;
        }
        // float x = (glm::sin(engine_instance->time_seconds()) + 1.f) / 2.f;
        // float y = (glm::cos(engine_instance->time_seconds()) + 1.f) / 2.f;
        // base_rt[0]->rhi_clear_color(Color(x, y, 0.f, 1.f));

        m_material->apply();
        m_vertex_buffer->rhi_bind(0);
        rhi->draw(6, 0);

        Rect2D rect;
        rect.position = {0, 0};
        rect.size     = viewport->size();
        viewport->rhi_bind();

        if (param)
        {
            reinterpret_cast<CombinedImageSampler2DMaterialParameter*>(param)->texture = base_rt[0];
        }

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
