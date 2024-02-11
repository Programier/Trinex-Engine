#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    // Using GLSL std140 alignment rules

    struct ENGINE_EXPORT GlobalShaderParameters {
        alignas(16) Matrix4f projection;
        alignas(16) Matrix4f view;

        alignas(16) Matrix4f projview;
        alignas(16) Matrix4f inv_projview;

        alignas(16) Vector4D viewport;
        alignas(16) Vector4D scissor;

        alignas(16) Vector3D camera_location;
        alignas(16) Vector3D camera_forward;
        alignas(16) Vector3D camera_right;
        alignas(16) Vector3D camera_up;

        alignas(8) Vector2D size;
        alignas(8) Vector2D depth_range;

        alignas(4) float time;
        alignas(4) float delta_time;
        alignas(4) float fov;
        alignas(4) float gamma;
        alignas(4) float aspect_ratio;


        GlobalShaderParameters& update(class RenderTargetBase* render_target, class CameraComponent* camera);
    };
}// namespace Engine
