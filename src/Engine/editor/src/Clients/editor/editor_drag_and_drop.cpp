#include <Clients/editor_client.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <Engine/world.hpp>
#include <Graphics/mesh.hpp>

namespace Engine
{
    EditorClient& EditorClient::on_object_dropped(Object* object)
    {
        if (StaticMesh* mesh = object->instance_cast<StaticMesh>())
        {
            StaticMeshActor* actor =
                    _M_world->spawn_actor(StaticMeshActor::static_class_instance())->instance_cast<StaticMeshActor>();
            actor->mesh_component()->mesh = mesh;
        }

        return *this;
    }
}// namespace Engine
