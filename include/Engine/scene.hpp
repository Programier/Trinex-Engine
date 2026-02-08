#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/flat_set.hpp>
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
	struct Frustum;

	class ENGINE_EXPORT Scene final
	{
	public:
		using PrimitiveOctree   = Octree<PrimitiveComponent*>;
		using LightOctree       = Octree<LightComponent*>;
		using PostProcessOctree = Octree<PostProcessComponent*>;

	private:
		PrimitiveOctree m_primitive_octree;
		LightOctree m_light_octree;
		PostProcessOctree m_post_process_octree;

		FlatSet<PostProcessComponent*> m_unbound_post_processes;
		FlatSet<LightComponent*> m_directional_lights;

	public:
		WorldEnvironment environment;

		Scene& add_primitive(PrimitiveComponent* primitive);
		Scene& remove_primitive(PrimitiveComponent* primitive);
		Scene& update_primitive_transform(PrimitiveComponent* primitive);

		Scene& add_light(LightComponent* light);
		Scene& remove_light(LightComponent* light);
		Scene& update_light_transform(LightComponent* light);

		Scene& add_post_process(PostProcessComponent* post_process);
		Scene& remove_post_process(PostProcessComponent* post_process);
		Scene& update_post_process_transform(PostProcessComponent* post_process);

		inline const PrimitiveOctree& primitive_octree() const { return m_primitive_octree; }
		inline const LightOctree& light_octree() const { return m_light_octree; };
		inline const PostProcessOctree& post_process_octree() const { return m_post_process_octree; }

		FrameVector<PrimitiveComponent*> collect_visible_primitives(const Frustum& frustum);
		FrameVector<LightComponent*> collect_visible_lights(const Frustum& frustum);
		FrameVector<PostProcessComponent*> collect_post_processes(const Vector3f& location);
		const Box3f& bounds() const;
	};
}// namespace Engine
