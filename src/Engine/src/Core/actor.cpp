#include <Core/actor.hpp>

namespace Engine
{
    Actor& Actor::update()
    {
        return *this;
    }

    Actor& Actor::load()
    {
        return *this;
    }

    Actor& Actor::unload()
    {
        return *this;
    }

    Actor& Actor::render()
    {
        return *this;
    }

    Actor& Actor::parent(Actor* actor)
    {
        Actor* parent_actor = _M_parent.ptr();
        if (parent_actor)
        {
            parent_actor->remove_child(this);
        }

        actor->child(this);
        return *this;
    }

    Actor* Actor::parent() const
    {
        return const_cast<Actor*>(_M_parent.ptr());
    }

    const Actor::ActorChilds& Actor::childs() const
    {
        return _M_childs;
    }

    Actor& Actor::child(Actor* actor)
    {
        if (actor->_M_parent && actor->_M_parent.ptr() != this)
        {
            actor->_M_parent->remove_child(actor);
        }

        _M_childs.insert(Pointer(actor));
        actor->_M_parent = this;
        return *this;
    }

    Actor& Actor::remove_child(Actor* actor)
    {
        if (actor->_M_parent.ptr() == this)
        {
            _M_childs.erase(Pointer(actor));
            actor->_M_parent = nullptr;
        }

        return *this;
    }
}// namespace Engine
