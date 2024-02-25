#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
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


namespace Engine
{
    void SceneRenderer::clear_render_targets(RenderTargetBase*, SceneLayer*)
    {
        GBuffer::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearGBuffer));
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearOneAttachentOutput));
    }

    void SceneRenderer::begin_rendering_base_pass(RenderTargetBase*, SceneLayer*)
    {
        begin_rendering_target(GBuffer::instance());
    }

    void SceneRenderer::begin_lighting_pass(RenderTargetBase*, SceneLayer*)
    {
        begin_rendering_target(SceneColorOutput::instance());

        Material* mat = Object::find_object_checked<Material>("DefaultPackage::BaseColorToScreenMat");
        PositionVertexBuffer* positions =
                Object::find_object_checked<PositionVertexBuffer>("DefaultPackage::ScreenPositionBuffer");

        if (mat)
        {
            mat->apply();
            positions->rhi_bind(0, 0);
            engine_instance->rhi()->draw(6);
        }
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

    SceneRenderer::SceneRenderer() : m_scene(nullptr)
    {}

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
