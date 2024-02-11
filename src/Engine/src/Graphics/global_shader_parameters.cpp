#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Graphics/global_shader_parameters.hpp>
#include <Graphics/render_target_base.hpp>


namespace Engine
{
    GlobalShaderParameters& GlobalShaderParameters::update(class RenderTargetBase* render_target, class CameraComponent* camera)
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

        if (camera)
        {
            const Transform& transform = camera->transform;
            Matrix3f mat3              = transform.local_to_world;
            camera_location            = mat3 * transform.location;
            camera_forward             = glm::normalize(mat3 * transform.forward_vector());
            camera_right               = glm::normalize(mat3 * transform.right_vector());
            camera_up                  = glm::normalize(mat3 * transform.up_vector());

            camera->aspect_ratio = size.x / size.y;

            projection = camera->projection_matrix();
            fov        = camera->fov;

            if (render_target)
            {
                view = glm::lookAt(camera_location, camera_location + camera_forward, camera_up);
            }

            aspect_ratio = camera->aspect_ratio;
        }

        projview     = projection * view;
        inv_projview = glm::inverse(projview);

        gamma      = engine_config.gamma;
        time       = engine_instance->time_seconds();
        delta_time = engine_instance->delta_time();
        return *this;
    }
}// namespace Engine
