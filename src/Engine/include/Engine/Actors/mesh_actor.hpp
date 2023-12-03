#pragma once

#include <Core/actor.hpp>
#include <Core/object_ref.hpp>

namespace Engine
{
    class ENGINE_EXPORT StaticMeshActor : public Actor
    {
    public:

        ObjectReference<class StaticMeshComponent> mesh_component;
    };
}
