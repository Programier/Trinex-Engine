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
        float ortho_geight;
        float near_clip_plane;
        float far_clip_plane;
        float aspect_ratio;

        bool archive_process(Archive* archive) override;
        CameraComponent& camera_view(CameraView& out);
    };
}// namespace Engine