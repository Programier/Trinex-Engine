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
			StaticMeshActor* actor = Object::new_instance<StaticMeshActor>();
			actor->mesh_component()->mesh(mesh);
			actor->owner(m_world);
		}

		return *this;
	}
}// namespace Engine
