#include <Core/base_engine.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader_parameters.hpp>


namespace Engine
{
    GlobalShaderParameters& GlobalShaderParameters::update(const SceneView* scene_view)
    {
        {
            ViewPort _viewport = rhi->viewport();
            size               = SceneRenderTargets::instance()->size();
            viewport           = {_viewport.pos.x, _viewport.pos.y, _viewport.size.x, _viewport.size.y};
            depth_range        = {_viewport.min_depth, _viewport.max_depth};
        }

        if (scene_view)
        {
            auto& camera    = scene_view->camera_view();
            camera_location = camera.location;
            camera_forward  = camera.forward_vector;
            camera_right    = camera.right_vector;
            camera_up       = camera.up_vector;

            projection = scene_view->projection_matrix();
            view       = scene_view->view_matrix();

            fov                    = camera.fov;
            ortho_width            = camera.ortho_width;
            ortho_height           = camera.ortho_height;
            near_clip_plane        = camera.near_clip_plane;
            far_clip_plane         = camera.far_clip_plane;
            aspect_ratio           = camera.aspect_ratio;
            camera_projection_mode = static_cast<int>(camera.projection_mode);

            projview     = scene_view->projview_matrix();
            inv_projview = scene_view->inv_projview_matrix();
        }

        gamma      = engine_instance->gamma();
        time       = engine_instance->time_seconds();
        delta_time = engine_instance->delta_time();
        return *this;
    }
}// namespace Engine
