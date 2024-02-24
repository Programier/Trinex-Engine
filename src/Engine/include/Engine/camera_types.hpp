#pragma once
#include <Core/engine_types.hpp>
#include <Core/transform.hpp>

namespace Engine
{
    enum class CameraProjectionMode
    {
        Perspective  = 0,
        Orthographic = 1,
    };

    struct CameraView {
        Vector3D location;
        Quaternion rotation;
        Vector3D forward_vector;
        Vector3D up_vector;
        Vector3D right_vector;

        CameraProjectionMode projection_mode;
        float fov;
        float ortho_width;
        float ortho_height;
        float near_clip_plane;
        float far_clip_plane;
        float aspect_ratio;

        Matrix4f projection_matrix() const;
        Matrix4f view_matrix() const;
        static Matrix4f view_matrix(const Vector3D& position, const Vector3D& direction, const Vector3D& up_vector);

        CameraView& operator=(class CameraComponent*);
        CameraView& operator=(class CameraComponent&);
    };
}// namespace Engine
