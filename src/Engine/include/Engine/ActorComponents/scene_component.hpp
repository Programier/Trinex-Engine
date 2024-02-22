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
        Transform transform_render_thread;

    private:
        Pointer<SceneComponent> m_parent = nullptr;
        Vector<Pointer<SceneComponent>> m_childs;

    public:
        SceneComponent();
        SceneComponent& attach(SceneComponent* child);
        SceneComponent& detach_from_parent();
        bool is_attached_to(SceneComponent* component) const;
        SceneComponent* parent() const;
        const Vector<Pointer<SceneComponent>>& childs() const;
        virtual SceneComponent& on_transform_changed();

        SceneComponent& destroyed() override;
    };
}// namespace Engine
