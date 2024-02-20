#pragma once

#include <Engine/Actors/actor.hpp>

namespace Engine
{
    class ENGINE_EXPORT StaticMeshActor : public Actor
    {
        declare_class(StaticMeshActor, Actor);

    private:
        class StaticMeshComponent* _M_mesh_component = nullptr;

    public:
        StaticMeshComponent* mesh_component() const;

        StaticMeshActor();
        ~StaticMeshActor();
    };
}// namespace Engine
