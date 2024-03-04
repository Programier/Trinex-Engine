#pragma once
#include <Core/pointer.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/actor_component.hpp>

namespace Engine
{
    class ENGINE_EXPORT SceneComponent : public ActorComponent
    {
        declare_class(SceneComponent, ActorComponent);

    private:
        mutable Transform m_local;
        mutable Transform m_world;
        mutable Transform m_local_render_thread;
        mutable Transform m_world_render_thread;
        mutable bool m_is_dirty = false;


        Pointer<SceneComponent> m_parent = nullptr;
        Vector<Pointer<SceneComponent>> m_childs;

        void mark_transfrom_dirty() const;

    public:
        SceneComponent();
        SceneComponent& attach(SceneComponent* child);
        SceneComponent& detach_from_parent();
        bool is_attached_to(SceneComponent* component) const;
        SceneComponent* parent() const;
        const Vector<Pointer<SceneComponent>>& childs() const;
        virtual SceneComponent& on_transform_changed();
        SceneComponent& destroyed() override;

        bool is_dirty_transform() const;
        const Transform& local_transform() const;
        const Transform& world_transform() const;

        SceneComponent& local_transform(const Transform&);
        SceneComponent& add_local_transform(const Transform&);
        SceneComponent& remove_local_transform(const Transform&);
        SceneComponent& location(const Vector3D&);
        SceneComponent& rotation(const Quaternion&);
        SceneComponent& rotation(const Vector3D&);
        SceneComponent& scale(const Vector3D&);
        SceneComponent& add_location(const Vector3D& delta);
        SceneComponent& add_rotation(const Vector3D& delta);
        SceneComponent& add_rotation(const Quaternion& delta);
        SceneComponent& add_scale(const Vector3D& delta);
    };
}// namespace Engine
