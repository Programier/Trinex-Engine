#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/transform.hpp>

namespace Engine
{
    enum class ActorFlags : EnumerateType
    {
        None      = 0,
        IsVisible = 1,

        __COUNT__
    };


    class ENGINE_EXPORT Actor : public Object
    {

    public:
        using ActorChilds = Set<Pointer<Actor>, Pointer<Actor>::HashStruct>;
        using Super       = Object;

    private:
        Pointer<Actor> _M_parent;
        ActorChilds _M_childs;

        BitSet<static_cast<size_t>(ActorFlags::__COUNT__)> _M_actor_flags;

    public:
        Transform transform;

        virtual Actor& update();
        virtual Actor& load();
        virtual Actor& unload();
        virtual Actor& render();

        Actor& parent(Actor* actor);
        Actor* parent() const;
        const ActorChilds& childs() const;
        Actor& child(Actor* actor);
        Actor& remove_child(Actor* actor);
    };
}// namespace Engine
