#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    SceneRenderer::SceneRenderer() : m_scene(nullptr), m_view_mode(ViewMode::Lit)
    {}

    void SceneRenderer::clear_render_targets(RenderTargetBase*, SceneLayer*)
    {
        GBuffer::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearGBuffer));
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearOneAttachentOutput));
    }

    void SceneRenderer::begin_rendering_base_pass(RenderTargetBase*, SceneLayer*)
    {
        begin_rendering_target(GBuffer::instance());
    }

    void SceneRenderer::copy_gbuffer_to_scene_output()
    {
        static Name screen_texture      = "screen_texture";
        Material* material              = DefaultResources::screen_material;
        PositionVertexBuffer* positions = DefaultResources::screen_position_buffer;

        if (material && positions)
        {
            using TextureParam = CombinedImageSampler2DMaterialParameter;

            if (TextureParam* texture = reinterpret_cast<TextureParam*>(material->find_parameter(screen_texture)))
            {
                Texture* base_color = reinterpret_cast<class Texture*>(GBuffer::instance()->base_color());
                texture->texture_param(base_color);
                material->apply();
                positions->rhi_bind(0, 0);
                engine_instance->rhi()->draw(6);
            }
        }
    }


    void SceneRenderer::begin_deferred_lighting_pass(RenderTargetBase* rt, SceneLayer* layer)
    {
        begin_rendering_target(SceneColorOutput::instance());

        if (m_view_mode == ViewMode::Unlit)
        {
            copy_gbuffer_to_scene_output();
        }
        else if (m_view_mode == ViewMode::Lit)
        {
            auto& components = layer->light_components();

            // Render only ambient light
            static Name name_ambient_color = "ambient_color";
            Material* material             = DefaultResources::ambient_light_material;

            if (material)
            {
                auto ambient_param = reinterpret_cast<Vec3MaterialParameter*>(material->find_parameter(name_ambient_color));

                if (ambient_param)
                {
                    ambient_param->param = m_scene->environment.ambient_color;
                }

                material->apply();
                DefaultResources::screen_position_buffer->rhi_bind(0, 0);
                engine_instance->rhi()->draw(6);
            }


            for (LightComponent* component : components)
            {
                component->render(this, rt, layer);
            }
        }
    }

    void SceneRenderer::begin_lighting_pass(RenderTargetBase*, SceneLayer*)
    {
        begin_rendering_target(SceneColorOutput::instance());
    }

    void SceneRenderer::begin_scene_output_pass(RenderTargetBase*, SceneLayer*)
    {
        begin_rendering_target(SceneColorOutput::instance());
    }

    void SceneRenderer::begin_postprocess_pass(RenderTargetBase*, SceneLayer*)
    {
        begin_rendering_target(SceneColorOutput::instance());
    }

    void SceneRenderer::end_rendering_target(RenderTargetBase*, SceneLayer*)
    {
        end_rendering_target();
    }

    SceneRenderer& SceneRenderer::end_rendering_target()
    {
        engine_instance->rhi()->pop_global_params();
        return *this;
    }

    SceneRenderer& SceneRenderer::view_mode(ViewMode new_mode)
    {
        if (is_in_render_thread())
        {
            m_view_mode = new_mode;
        }
        else
        {
            call_in_render_thread([this, new_mode]() { m_view_mode = new_mode; });
        }
        return *this;
    }


    SceneRenderer& SceneRenderer::scene(Scene* scene)
    {
        m_scene = scene;
        return *this;
    }

    Scene* SceneRenderer::scene() const
    {
        return m_scene;
    }

    SceneRenderer& SceneRenderer::begin_rendering_target(RenderTargetBase* render_target, RenderPass* render_pass)
    {
        setup_parameters(render_target, nullptr);
        RHI* rhi = engine_instance->rhi();
        rhi->push_global_params(m_global_shader_params);
        render_target->rhi_bind(render_pass);
        return *this;
    }

    SceneRenderer& SceneRenderer::setup_parameters(RenderTargetBase* render_target, SceneView* scene_view)
    {
        if (render_target)
        {
            ViewPort viewport;
            viewport.size      = m_scene_view.view_size();
            viewport.pos       = {0, 0};
            viewport.min_depth = 0.f;
            viewport.max_depth = 1.f;

            Scissor scissor;
            scissor.size = m_scene_view.view_size();
            scissor.pos  = {0, 0};

            render_target->viewport(viewport);
            render_target->scissor(scissor);
        }

        m_global_shader_params.update(render_target, scene_view);
        return *this;
    }

    SceneRenderer& SceneRenderer::render(const SceneView& view, RenderTargetBase* render_target)
    {
        if (m_scene == nullptr)
            return *this;

        m_scene_view = view;
        setup_parameters(nullptr, &m_scene_view);

        m_scene->build_views(this);
        auto rhi = engine_instance->rhi();

        for (auto layer = m_scene->root_layer(); layer; layer = layer->next())
        {
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage(layer->name().c_str());
#endif
            layer->render(this, render_target);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }

        return *this;
    }

    SceneRenderer::~SceneRenderer()
    {}
}// namespace Engine
