#pragma once

#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/camera_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT CameraComponent : public SceneComponent
    {
        declare_class(CameraComponent, SceneComponent);

    public:
        CameraProjectionMode projection_mode;
        float fov;
        float ortho_width;
        float ortho_height;
        float near_clip_plane;
        float far_clip_plane;
        float aspect_ratio;

        bool archive_process(Archive& archive) override;
        CameraComponent& camera_view(CameraView& out);
        Matrix4f projection_matrix();
        static Matrix4f view_matrix(const Vector3D& position, const Vector3D& direction, const Vector3D& up_vector);
    };
}// namespace Engine
