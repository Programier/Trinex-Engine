#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/logger.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/command_buffer.hpp>
#include <Engine/Render/rendering_policy.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_output_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    SceneRenderer::SceneRenderer() : scene(nullptr)
    {
        m_root_layer = new RootLayer();
    }

    const GlobalShaderParameters& SceneRenderer::global_shader_parameters() const
    {
        if (m_global_shader_params.empty())
            throw EngineException("Shader parameters stack is empty!");
        return m_global_shader_params.back();
    }

    const SceneView& SceneRenderer::scene_view() const
    {
        if (m_scene_views.empty())
            throw EngineException("Scene Views stack is empty!");
        return m_scene_views.back();
    }


    SceneRenderer& SceneRenderer::push_global_shader_parameters()
    {
        const SceneView& view = scene_view();
        m_global_shader_params.emplace_back();
        m_global_shader_params.back().update(&view);
        rhi->push_global_params(m_global_shader_params.back());
        return *this;
    }

    SceneRenderer& SceneRenderer::pop_global_shader_parameters()
    {
        rhi->pop_global_params();
        m_global_shader_params.pop_back();
        return *this;
    }

    RenderSurface* SceneRenderer::output_surface() const
    {
        return SceneRenderTargets::instance()->surface_of(SceneRenderTargets::SceneColorLDR);
    }

    SceneRenderer& SceneRenderer::render(const SceneView& view, RenderViewport* viewport)
    {
        if (scene == nullptr)
            return *this;

        m_global_shader_params.clear();
        m_scene_views.clear();
        m_scene_views.push_back(view);

        rhi->viewport(view.viewport());
        rhi->scissor(view.scissor());

        push_global_shader_parameters();

        for (auto layer = root_layer(); layer; layer = layer->next())
        {
            layer->clear();
        }

        scene->build_views(this);

        for (auto layer = root_layer(); layer; layer = layer->next())
        {
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage(layer->name().c_str());
#endif
            layer->begin_render(this, viewport);
            layer->render(this, viewport);
            layer->end_render(this, viewport);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }

        pop_global_shader_parameters();
        m_scene_views.pop_back();

        return *this;
    }

    SceneRenderer& SceneRenderer::render_component(PrimitiveComponent* component)
    {
        return *this;
    }

    SceneRenderer& SceneRenderer::render_component(LightComponent* component)
    {
        return *this;
    }

    SceneRenderer::~SceneRenderer()
    {
        delete m_root_layer;
        m_root_layer = nullptr;
    }

#define declare_rendering_function() [](SceneRenderer * self, RenderViewport * viewport, SceneLayer * layer)

    static void copy_gbuffer_to_scene_output()
    {
        // static Name screen_texture      = "screen_texture";
        // Material* material              = DefaultResources::screen_material;
        // PositionVertexBuffer* positions = DefaultResources::screen_position_buffer;

        // if (material && positions)
        // {
        //     using TextureParam = CombinedImageSampler2DMaterialParameter;

        //     if (TextureParam* texture = reinterpret_cast<TextureParam*>(material->find_parameter(screen_texture)))
        //     {
        //         Texture* base_color = reinterpret_cast<class Texture*>(GBuffer::instance()->base_color());
        //         texture->texture_param(base_color);
        //         material->apply();
        //         positions->rhi_bind(0, 0);
        //         rhi->draw(6, 0);
        //     }
        // }
    }

    static void render_ambient_light_only(Scene* scene)
    {
        static Name name_ambient_color = "ambient_color";
        Material* material             = DefaultResources::ambient_light_material;

        if (material)
        {
            auto ambient_param = reinterpret_cast<Vec3MaterialParameter*>(material->find_parameter(name_ambient_color));

            if (ambient_param)
            {
                ambient_param->param = scene->environment.ambient_color;
            }

            material->apply();
            DefaultResources::screen_position_buffer->rhi_bind(0, 0);
            rhi->draw(6, 0);
        }
    }

    static void begin_deferred_lighting_pass(SceneRenderer* _self, RenderViewport* rt, SceneLayer* layer)
    {
        ColorSceneRenderer* self = reinterpret_cast<ColorSceneRenderer*>(_self);

        if (self->view_mode() == ViewMode::Unlit)
        {
            copy_gbuffer_to_scene_output();
        }
        else if (self->view_mode() == ViewMode::Lit)
        {
            render_ambient_light_only(self->scene);
        }
    }

    class DeferredLightingLayer : public CommandBufferLayer
    {
    public:
        DeferredLightingLayer& render(SceneRenderer* renderer, RenderViewport* viewport)
        {
            if (reinterpret_cast<ColorSceneRenderer*>(renderer)->view_mode() == ViewMode::Lit)
            {
                CommandBufferLayer::render(renderer, viewport);
            }

            return *this;
        }
    };

    ColorSceneRenderer::ColorSceneRenderer() : m_view_mode(ViewMode::Lit)
    {
        m_clear_layer = root_layer()->create_next(Name::clear_render_targets);
        m_clear_layer->on_begin_render.push_back(declare_rendering_function() { SceneRenderTargets::instance()->clear(); });


        m_depth_layer = m_clear_layer->create_next<DepthRenderingLayer>(Name::depth_pass);

        m_base_pass_layer = m_depth_layer->create_next<CommandBufferLayer>(Name::base_pass);
        m_base_pass_layer->on_begin_render.push_back(
                declare_rendering_function() { SceneRenderTargets::instance()->begin_rendering_gbuffer(); });
        m_base_pass_layer->on_end_render.push_back(
                declare_rendering_function() { SceneRenderTargets::instance()->end_rendering_gbuffer(); });

        m_deferred_lighting_layer = m_base_pass_layer->create_next<DeferredLightingLayer>(Name::deferred_light_pass);
        m_deferred_lighting_layer->on_begin_render.push_back(
                declare_rendering_function() { SceneRenderTargets::instance()->begin_rendering_scene_color_ldr(); });
        m_deferred_lighting_layer->on_begin_render.push_back(begin_deferred_lighting_pass);
        m_base_pass_layer->on_end_render.push_back(
                declare_rendering_function() { SceneRenderTargets::instance()->end_rendering_scene_color_ldr(); });

        m_scene_output       = m_deferred_lighting_layer->create_next<SceneOutputLayer>(Name::scene_output_pass);
        m_post_process_layer = m_scene_output->create_next(Name::post_process);
    }

    DepthSceneRenderer* ColorSceneRenderer::create_depth_renderer()
    {
        return new DepthSceneRenderer();
    }

    ColorSceneRenderer::~ColorSceneRenderer()
    {
        if (m_depth_renderer)
        {
            delete m_depth_renderer;
        }
    }

    PolicyID ColorSceneRenderer::policy_id()
    {
        static PolicyID id = ::Engine::policy_id(Name::color_scene_rendering);
        return id;
    }

    ColorSceneRenderer& ColorSceneRenderer::view_mode(ViewMode new_mode)
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


}// namespace Engine
