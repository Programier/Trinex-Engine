#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Window/window.hpp>
#include <functional>
#include <glm/ext.hpp>

namespace Engine
{
    implement_class(CameraComponent, Engine, 0);
    implement_initialize_class(CameraComponent)
    {}

    bool CameraComponent::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
            return false;

        (*archive) & projection_mode;
        (*archive) & fov;
        (*archive) & ortho_width;
        (*archive) & ortho_height;
        (*archive) & near_clip_plane;
        (*archive) & far_clip_plane;
        (*archive) & aspect_ratio;


        return static_cast<bool>(*archive);
    }

    Matrix4f CameraComponent::projection_matrix()
    {
        if (projection_mode == CameraProjectionMode::Perspective)
        {
            return glm::perspective(fov, aspect_ratio, near_clip_plane, far_clip_plane);
        }
        else if (projection_mode == CameraProjectionMode::Orthographic)
        {
            return glm::ortho(-ortho_width / 2.0f, // Left
                              ortho_width / 2.0f,  // Right
                              -ortho_height / 2.0f,// Bottom
                              ortho_height / 2.0f, // Top
                              near_clip_plane,     // Near clipping plane
                              far_clip_plane       // Far clipping plane
            );
        }

        return glm::mat4(1.0);
    }

    Matrix4f CameraComponent::view_matrix(const Vector3D& position, const Vector3D& direction, const Vector3D& up_vector)
    {
        return glm::lookAt(position, direction, up_vector);
    }

    CameraComponent& CameraComponent::camera_view(CameraView& out)
    {
        out.location        = transform.location;
        out.rotation        = transform.rotation;
        out.projection_mode = projection_mode;
        out.fov             = fov;
        out.ortho_width     = ortho_width;
        out.ortho_height    = ortho_height;
        out.near_clip_plane = near_clip_plane;
        out.far_clip_plane  = far_clip_plane;
        out.aspect_ratio    = aspect_ratio;

        return *this;
    }

    CameraView& CameraView::operator=(class CameraComponent* component)
    {
        if (component)
        {
            component->camera_view(*this);
        }
        return *this;
    }

    CameraView& CameraView::operator=(class CameraComponent& component)
    {
        component.camera_view(*this);
        return *this;
    }
}// namespace Engine
