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
			        m_world->spawn_actor(StaticMeshActor::static_reflection())->instance_cast<StaticMeshActor>();
			actor->mesh_component()->mesh(mesh);
			actor->mesh_component()->on_transform_changed();
		}

		return *this;
	}
}// namespace Engine
