#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
    void SceneRenderer::clear_render_targets(RenderViewport*, SceneLayer*)
    {
        GBuffer::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearGBuffer));
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearSceneOutput));
    }

    SceneRenderer::SceneRenderer() : _M_scene(nullptr)
    {}

    SceneRenderer& SceneRenderer::scene(Scene* scene)
    {
        _M_scene = scene;
        return *this;
    }

    Scene* SceneRenderer::scene() const
    {
        return _M_scene;
    }

    SceneRenderer& SceneRenderer::render(const CameraView& view, RenderViewport* viewport, const Size2D& size)
    {
        if (_M_scene == nullptr)
            return *this;

        _M_size        = size;
        _M_camera_view = view;

        _M_scene->build_views(this);

#if TRINEX_DEBUG_BUILD
        auto rhi = engine_instance->rhi();
#endif
        for (auto layer = _M_scene->root_layer(); layer; layer = layer->next())
        {
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage(layer->name().c_str());
#endif
            layer->render(this, viewport);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }
        return *this;
    }

    const SceneRenderer& SceneRenderer::screen_to_world(const Vector2D& screen_point, Vector3D& world_origin,
                                                        Vector3D& world_direction) const
    {
        int32_t x = glm::trunc(screen_point.x), y = glm::trunc(screen_point.y);

        Matrix4f inverse_view       = glm::inverse(_M_view);
        Matrix4f inverse_projection = glm::inverse(_M_projection);

        float screen_space_x                = (x - _M_size.x / 2.f) / (_M_size.x / 2.f);
        float screen_space_y                = (y - _M_size.y / 2.f) / -(_M_size.y / 2.f);
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
        return _M_projview * Vector4D(world_point, 1.f);
    }

    SceneRenderer::~SceneRenderer()
    {}
}// namespace Engine
