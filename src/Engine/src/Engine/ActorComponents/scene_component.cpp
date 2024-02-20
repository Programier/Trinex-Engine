#include <Core/class.hpp>
#include <Core/exception.hpp>
#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/scene_component.hpp>


namespace Engine
{
    implement_engine_class_default_init(SceneComponent);

    SceneComponent::SceneComponent()
    {}

    SceneComponent& SceneComponent::attach(SceneComponent* child)
    {
        trinex_check(child != this, "Cannot attach a component to itself");
        trinex_check(child && !is_attached_to(child), "Setting up attachment would create a cycle");

        detach_from_parent();

        child->_M_parent = this;
        _M_childs.push_back(child);
        return *this;
    }

    SceneComponent& SceneComponent::detach_from_parent()
    {
        if (_M_parent)
        {
            for (size_t index = 0, count = _M_parent->_M_childs.size(); index < count; ++index)
            {
                SceneComponent* component = _M_parent->_M_childs[index];
                if (component == this)
                {
                    _M_parent->_M_childs.erase(_M_parent->_M_childs.begin() + index);
                    break;
                }
            }

            _M_parent = nullptr;
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
        return _M_childs;
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
        transform.update(parent());

        render_thread()->insert_new_task<UpdateComponentTransform>(transform, transform_render_thread);

        for (SceneComponent* child : _M_childs)
        {
            child->on_transform_changed();
        }

        return *this;
    }

    SceneComponent* SceneComponent::parent() const
    {
        return _M_parent.ptr();
    }

    SceneComponent& SceneComponent::destroyed()
    {
        detach_from_parent();
        Super::destroyed();

        return *this;
    }
}// namespace Engine
