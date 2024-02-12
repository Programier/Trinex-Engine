#pragma once
#include <Core/pointer.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/actor_component.hpp>

namespace Engine
{
    class ENGINE_EXPORT SceneComponent : public ActorComponent
    {
        declare_class(SceneComponent, ActorComponent);

    public:
        Transform transform;

    private:
        Pointer<SceneComponent> _M_parent = nullptr;
        Vector<Pointer<SceneComponent>> _M_childs;

    public:
        SceneComponent();
        SceneComponent& attach(SceneComponent* child);
        SceneComponent& detach_from_parent();
        bool is_attachet_to(SceneComponent* component) const;
        SceneComponent* parent() const;
        const Vector<Pointer<SceneComponent>>& childs() const;

        SceneComponent& destroyed() override;
    };
}// namespace Engine
