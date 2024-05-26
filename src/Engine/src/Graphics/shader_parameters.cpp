#include <Core/base_engine.hpp>
#include <Core/engine_config.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/shader_parameters.hpp>


namespace Engine
{

    StringView GlobalShaderParameters::shader_code()
    {
        return R"(#ifndef PROJECTION_MODE_PERSPECTIVE
#define PROJECTION_MODE_PERSPECTIVE 0
#endif

#ifndef PROJECTION_MODE_ORTHOGRAPHIC
#define PROJECTION_MODE_ORTHOGRAPHIC 1
#endif

layout(binding = 0, std140) uniform _Global
{
    layout(column_major) mat4 projection;
    layout(column_major) mat4 view;

    layout(column_major) mat4 projview;
    layout(column_major) mat4 inv_projview;

    vec4 viewport;
    vec4 scissor;

    vec3 camera_location;
    vec3 camera_forward;
    vec3 camera_right;
    vec3 camera_up;

    vec2 size;
    vec2 depth_range;

    float gamma;
    float time;
    float delta_time;

    float fov;
    float ortho_width;
    float ortho_height;
    float near_clip_plane;
    float far_clip_plane;
    float aspect_ratio;
    int camera_projection_mode;
})";
    }

    GlobalShaderParameters& GlobalShaderParameters::update(const class RenderTargetBase* render_target,
                                                           const SceneView* scene_view)
    {
        if (render_target)
        {
            size            = render_target->render_target_size();
            auto& _viewport = render_target->viewport();
            auto& _scissor  = render_target->scissor();
            viewport        = {_viewport.pos.x, _viewport.pos.y, _viewport.size.x, _viewport.size.y};
            scissor         = {_scissor.pos.x, _scissor.pos.y, _scissor.size.x, _scissor.size.y};
            depth_range     = {_viewport.min_depth, _viewport.max_depth};
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
