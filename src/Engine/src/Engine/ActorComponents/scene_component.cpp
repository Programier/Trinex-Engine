#include <Core/exception.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Core/class.hpp>


namespace Engine
{
    implement_engine_class_default_init(SceneComponent);

    SceneComponent& SceneComponent::attach(SceneComponent* parent)
    {
        trinex_check(parent != this, "Cannot attach a component to itself");
        trinex_check(parent && !parent->is_attachet_to(this), "Setting up attachment would create a cycle");

        detach_from_parent();

        _M_parent = parent;
        _M_parent->_M_childs.push_back(this);
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

    bool SceneComponent::is_attachet_to(SceneComponent* component) const
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
