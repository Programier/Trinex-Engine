#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <Core/name.hpp>
#include <Core/pointer.hpp>
#include <Engine/enviroment.hpp>
#include <Engine/octree.hpp>

namespace Engine
{
	class PrimitiveComponent;
	class LightComponent;
	class PostProcessComponent;
	class SceneComponent;
	struct Frustum;

	class ENGINE_EXPORT Scene final
	{
	private:
		using PrimitiveOctree   = Octree<PrimitiveComponent*>;
		using LightOctree       = Octree<LightComponent*>;
		using PostProcessOctree = Octree<PostProcessComponent*>;

		PrimitiveOctree m_primitive_octree;
		LightOctree m_light_octree;
		PostProcessOctree m_post_process_octree;
		Set<PostProcessComponent*> m_unbound_post_processes;

		Pointer<SceneComponent> m_root_component;

	public:
		WorldEnvironment environment;

		Scene();

		Scene& add_primitive(PrimitiveComponent* primitive);
		Scene& remove_primitive(PrimitiveComponent* primitive);
		Scene& update_primitive_transform(PrimitiveComponent* primitive);

		Scene& add_light(LightComponent* light);
		Scene& remove_light(LightComponent* light);
		Scene& update_light_transform(LightComponent* light);

		Scene& add_post_process(PostProcessComponent* post_process);
		Scene& remove_post_process(PostProcessComponent* post_process);
		Scene& update_post_process_transform(PostProcessComponent* post_process);

		SceneComponent* root_component() const;

		// Next methods can be used only from render thread
		FrameVector<PrimitiveComponent*> collect_visible_primitives(const Frustum& frustum);
		FrameVector<LightComponent*> collect_visible_lights(const Frustum& frustum);
		FrameVector<PostProcessComponent*> collect_post_processes(const Vector3f& location);
	};
}// namespace Engine
