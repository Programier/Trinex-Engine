#include <Core/class.hpp>
#include <Core/exception.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/scene_component.hpp>


namespace Engine
{
    implement_engine_class_default_init(SceneComponent);

    void SceneComponent::mark_transfrom_dirty() const
    {
        m_is_dirty = true;

        for (SceneComponent* child : childs())
        {
            child->mark_transfrom_dirty();
        }
    }

    SceneComponent::SceneComponent()
    {}

    SceneComponent& SceneComponent::attach(SceneComponent* child)
    {
        trinex_check(child != this, "Cannot attach a component to itself");
        trinex_check(child && !is_attached_to(child), "Setting up attachment would create a cycle");

        detach_from_parent();

        child->m_parent = this;
        m_childs.push_back(child);
        return *this;
    }

    SceneComponent& SceneComponent::detach_from_parent()
    {
        if (m_parent)
        {
            for (size_t index = 0, count = m_parent->m_childs.size(); index < count; ++index)
            {
                SceneComponent* component = m_parent->m_childs[index];
                if (component == this)
                {
                    m_parent->m_childs.erase(m_parent->m_childs.begin() + index);
                    break;
                }
            }

            m_parent = nullptr;
        }

        return *this;
    }

    bool SceneComponent::is_attached_to(SceneComponent* component) const
    {
        if (component != nullptr)
        {
            for (const SceneComponent* comp = parent(); comp != nullptr; comp = comp->parent())
            {
                if (comp == component)
                {
                    return true;
                }
            }
        }

        return false;
    }

    const Vector<Pointer<SceneComponent>>& SceneComponent::childs() const
    {
        return m_childs;
    }

    class UpdateComponentTransform : public ExecutableObject
    {
        Transform transform;
        Transform& dst_transform;

    public:
        UpdateComponentTransform(const Transform& src, Transform& dst) : transform(src), dst_transform(dst)
        {}

        int_t execute()
        {
            dst_transform = transform;
            return sizeof(UpdateComponentTransform);
        }
    };

    SceneComponent& SceneComponent::on_transform_changed()
    {
        if (is_dirty_transform())
        {
            SceneComponent* parent_component = parent();

            m_local.matrix();
            if (parent_component)
            {
                m_world = parent_component->world_transform() * m_local;
            }
            else
            {
                m_world = m_local;
            }

            render_thread()->insert_new_task<UpdateComponentTransform>(m_local, m_local_render_thread);
            render_thread()->insert_new_task<UpdateComponentTransform>(m_world, m_world_render_thread);

            m_is_dirty = false;
        }

        for (SceneComponent* child : m_childs)
        {
            child->on_transform_changed();
        }

        return *this;
    }

    SceneComponent* SceneComponent::parent() const
    {
        return m_parent.ptr();
    }

    SceneComponent& SceneComponent::destroyed()
    {
        detach_from_parent();
        Super::destroyed();

        return *this;
    }

    bool SceneComponent::is_dirty_transform() const
    {
        if (is_in_render_thread())
        {
            return false;
        }
        else
        {
            return m_is_dirty || m_local.is_dirty();
        }
    }

    const Transform& SceneComponent::local_transform() const
    {
        if (is_dirty_transform())
        {
            const_cast<SceneComponent*>(this)->on_transform_changed();
        }

        if (is_in_render_thread())
        {
            return m_local_render_thread;
        }
        else
        {
            return m_local;
        }
    }

    const Transform& SceneComponent::world_transform() const
    {
        if (is_dirty_transform())
        {
            const_cast<SceneComponent*>(this)->on_transform_changed();
        }

        if (is_in_render_thread())
        {
            return m_world_render_thread;
        }
        else
        {
            return m_world;
        }
    }

    SceneComponent& SceneComponent::local_transform(const Transform& transform)
    {
        if (is_in_logic_thread())
        {
            m_local = transform;
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::add_local_transform(const Transform& transform)
    {
        if (is_in_logic_thread())
        {
            m_local += transform;
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::remove_local_transform(const Transform& transform)
    {
        if (is_in_logic_thread())
        {
            m_local -= transform;
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::location(const Vector3D& new_location)
    {
        if (is_in_logic_thread())
        {
            m_local.location(new_location);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::rotation(const Quaternion& new_rotation)
    {
        if (is_in_logic_thread())
        {
            m_local.rotation(new_rotation);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::rotation(const Vector3D& new_rotation)
    {
        if (is_in_logic_thread())
        {
            m_local.rotation(new_rotation);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::scale(const Vector3D& new_scale)
    {
        if (is_in_logic_thread())
        {
            m_local.scale(new_scale);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::add_location(const Vector3D& delta)
    {
        if (is_in_logic_thread())
        {
            m_local.add_location(delta);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::add_rotation(const Vector3D& delta)
    {
        if (is_in_logic_thread())
        {
            m_local.add_rotation(delta);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::add_rotation(const Quaternion& delta)
    {
        if (is_in_logic_thread())
        {
            m_local.add_rotation(delta);
            mark_transfrom_dirty();
        }
        return *this;
    }

    SceneComponent& SceneComponent::add_scale(const Vector3D& delta)
    {
        if (is_in_logic_thread())
        {
            m_local.add_scale(delta);
            mark_transfrom_dirty();
        }
        return *this;
    }


}// namespace Engine
