#include <Core/actor.hpp>
#include <Core/buffer_manager.hpp>

namespace Engine
{
    Actor& Actor::update()
    {
        if (script.on_update.is_valid())
        {
            script.on_update(class_instance()->to_lua_object(this));
        }

        return *this;
    }

    Actor& Actor::load()
    {
        script.load();
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

    Actor& Actor::ready()
    {
        if (script.on_ready.is_valid())
        {
            script.on_ready(class_instance()->to_lua_object(this));
        }

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

    bool Actor::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
            return false;

        (*archive) & transform;
        (*archive) & script;

        return static_cast<bool>(*archive);
    }

    register_class(Engine::Actor)("update", &Actor::update,//
                                  "load", &Actor::load,    //
                                  "unload", &Actor::unload,//
                                  "render", &Actor::render,//
                                  "parent",
                                  Lua::overload(static_cast<Actor& (Actor::*) (Actor*)>(&Actor::parent),
                                                static_cast<Actor* (Actor::*) () const>(&Actor::parent)),
                                  "childs", &Actor::childs,            //
                                  "child", &Actor::child,              //
                                  "remove_child", &Actor::remove_child,//
                                  "transform", &Actor::transform);
}// namespace Engine
