#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class CameraProjectionMode
    {
        Perspective  = 0,
        Orthographic = 1,
    };

    struct CameraView {
        Vector3D location;
        Vector3D rotation;
        CameraProjectionMode projection_mode;
        float fov;
        float ortho_width;
        float ortho_height;
        float near_clip_plane;
        float far_clip_plane;
        float aspect_ratio;


        CameraView& operator=(class CameraComponent*);
        CameraView& operator=(class CameraComponent&);
    };
}// namespace Engine
