#include <Core/class.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>

namespace Engine
{
    implement_engine_class_default_init(PrimitiveComponent);

    bool PrimitiveComponent::is_visible() const
    {
        return _M_is_visible;
    }

    const AABB_3Df& PrimitiveComponent::bounding_box() const
    {
        return _M_bounding_box;
    }

    PrimitiveComponent& PrimitiveComponent::spawned()
    {
        return* this;
    }

    PrimitiveComponent& PrimitiveComponent::destroyed()
    {
        return* this;
    }
}// namespace Engine
