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
        Quaternion rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
        Vector3D position   = Vector3D(0.0f);
        Vector3D scale      = Vector3D(1.0f);

    private:
        Pointer<SceneComponent> _M_parent = nullptr;
        Vector<Pointer<SceneComponent>> _M_childs;

    public:
        SceneComponent& attach(SceneComponent* parent);
        SceneComponent& detach_from_parent();
        bool is_attachet_to(SceneComponent* component) const;
        SceneComponent* parent() const;
        const Vector<Pointer<SceneComponent>>& childs() const;

        SceneComponent& destroyed() override;
    };
}// namespace Engine
