#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
    class ENGINE_EXPORT PrimitiveDrawingProxy
    {
    public:
        virtual PrimitiveDrawingProxy& render(class PrimitiveComponent* component) = 0;
        virtual ~PrimitiveDrawingProxy();
    };

    class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
    {
        declare_class(PrimitiveComponent, SceneComponent);

    protected:
        bool _M_is_visible;
        AABB_3Df _M_bounding_box;

    public:
        bool is_visible() const;
        const AABB_3Df& bounding_box() const;

        PrimitiveComponent& spawned() override;
        PrimitiveComponent& destroyed() override;
        virtual PrimitiveDrawingProxy* drawing_proxy() const;
    };
}// namespace Engine
