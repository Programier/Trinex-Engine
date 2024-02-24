#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
    void SceneRenderer::clear_render_targets(RenderViewport*, SceneLayer*)
    {
        GBuffer::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearGBuffer));
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearSceneOutput));
    }

    void SceneRenderer::begin_rendering_base_pass(RenderViewport*, SceneLayer*)
    {
        GBuffer::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::GBuffer));
    }

    void SceneRenderer::begin_lighting_pass(RenderViewport*, SceneLayer*)
    {
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::SceneOutput));

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

    void SceneRenderer::begin_scene_output_pass(RenderViewport*, SceneLayer*)
    {
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::SceneOutput));
    }

    void SceneRenderer::begin_postprocess_pass(RenderViewport*, SceneLayer*)
    {
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::SceneOutput));
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


    SceneRenderer& SceneRenderer::setup_viewport(RenderViewport* render_viewport)
    {
        m_global_shader_params.update(render_viewport->base_render_target(), &m_camera_view);


        ViewPort viewport;
        viewport.size      = m_size;
        viewport.pos       = {0, 0};
        viewport.min_depth = 0.f;
        viewport.max_depth = 1.f;

        Scissor scissor;
        scissor.size = m_size;
        scissor.pos  = {0, 0};


        GBuffer* gbuffer                     = GBuffer::instance();
        SceneColorOutput* scene_color_output = SceneColorOutput::instance();

        gbuffer->viewport(viewport);
        gbuffer->scissor(scissor);

        scene_color_output->viewport(viewport);
        scene_color_output->scissor(scissor);

        RenderTargetBase* base_render_target = render_viewport->base_render_target();

        if (base_render_target != gbuffer && base_render_target != scene_color_output)
        {
            base_render_target->viewport(viewport);
            base_render_target->scissor(scissor);
        }

        return *this;
    }

    SceneRenderer& SceneRenderer::render(const CameraView& view, RenderViewport* viewport, const Size2D& size)
    {
        if (m_scene == nullptr)
            return *this;

        m_size        = size;
        m_camera_view = view;
        setup_viewport(viewport);

        m_scene->build_views(this);

        auto rhi = engine_instance->rhi();

        rhi->push_global_params(m_global_shader_params);

        for (auto layer = m_scene->root_layer(); layer; layer = layer->next())
        {
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage(layer->name().c_str());
#endif
            layer->render(this, viewport);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }

        rhi->pop_global_params();

        return *this;
    }

    const SceneRenderer& SceneRenderer::screen_to_world(const Vector2D& screen_point, Vector3D& world_origin,
                                                        Vector3D& world_direction) const
    {
        int32_t x = glm::trunc(screen_point.x), y = glm::trunc(screen_point.y);

        Matrix4f inverse_view       = glm::inverse(m_view);
        Matrix4f inverse_projection = glm::inverse(m_projection);

        float screen_space_x                = (x - m_size.x / 2.f) / (m_size.x / 2.f);
        float screen_space_y                = (y - m_size.y / 2.f) / -(m_size.y / 2.f);
        Vector4D ray_start_projection_space = Vector4D(screen_space_x, screen_space_y, 0.f, 1.0f);
        Vector4D ray_end_projection_space   = Vector4D(screen_space_x, screen_space_y, 0.5f, 1.0f);

        Vector4D hg_ray_start_view_space = inverse_projection * ray_start_projection_space;
        Vector4D hg_ray_end_view_space   = inverse_projection * ray_end_projection_space;

        Vector3D ray_start_view_space(hg_ray_start_view_space.x, hg_ray_start_view_space.y, hg_ray_start_view_space.z);
        Vector3D ray_end_view_space(hg_ray_end_view_space.x, hg_ray_end_view_space.y, hg_ray_end_view_space.z);

        if (hg_ray_start_view_space.w != 0.0f)
        {
            ray_start_view_space /= hg_ray_start_view_space.w;
        }
        if (hg_ray_end_view_space.w != 0.0f)
        {
            ray_end_view_space /= hg_ray_end_view_space.w;
        }

        Vector3D ray_dir_view_space = ray_end_view_space - ray_start_view_space;
        ray_dir_view_space          = glm::normalize(ray_dir_view_space);

        Vector3D ray_start_world_space = inverse_view * Vector4D(ray_start_view_space, 1.f);
        Vector3D ray_dir_world_space   = inverse_view * Vector4D(ray_dir_view_space, 0.f);

        world_origin    = ray_start_world_space;
        world_direction = glm::normalize(ray_dir_world_space);
        return *this;
    }

    Vector4D SceneRenderer::world_to_screen(const Vector3D& world_point) const
    {
        return m_projview * Vector4D(world_point, 1.f);
    }

    SceneRenderer::~SceneRenderer()
    {}
}// namespace Engine
