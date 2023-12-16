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
    implement_class(CameraComponent, "Engine", 0);
    implement_initialize_class(CameraComponent)
    {}

    bool CameraComponent::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
            return false;

        (*archive) & projection_mode;
        (*archive) & fov;
        (*archive) & ortho_width;
        (*archive) & ortho_geight;
        (*archive) & near_clip_plane;
        (*archive) & far_clip_plane;
        (*archive) & aspect_ratio;


        return static_cast<bool>(*archive);
    }

    CameraComponent& CameraComponent::camera_view(CameraView& out)
    {
        out.position        = position;
        out.rotation        = rotation;
        out.projection_mode = projection_mode;
        out.fov             = fov;
        out.ortho_width     = ortho_width;
        out.ortho_geight    = ortho_geight;
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
