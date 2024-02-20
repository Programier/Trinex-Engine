#include <Core/class.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>

namespace Engine
{
    implement_class(StaticMeshActor, Engine, 0);
    implement_initialize_class(StaticMeshActor)
    {
        Class* self = This::static_class_instance();
        self->add_property(new ObjectProperty("StaticMeshComponent0", "StaticMeshComponent0", &This::_M_mesh_component));
    }

    StaticMeshActor::StaticMeshActor()
    {
        _M_mesh_component = create_component<StaticMeshComponent>("StaticMeshComponent0");
    }

    StaticMeshComponent* StaticMeshActor::mesh_component() const
    {
        return _M_mesh_component;
    }

    StaticMeshActor::~StaticMeshActor()
    {}
}// namespace Engine
