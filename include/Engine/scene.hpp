#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/etl/vector.hpp>
#include <Core/name.hpp>
#include <Core/pointer.hpp>
#include <Engine/enviroment.hpp>
#include <Engine/octree.hpp>

namespace Trinex
{
	class PrimitiveComponent;
	class LightComponent;
	class PostProcessComponent;
	struct Frustum;

	class ENGINE_EXPORT Scene final
	{
	private:
		Octree m_primitive_octree;
		Octree m_light_octree;
		Octree m_post_process_octree;

		FlatSet<PostProcessComponent*> m_unbound_post_processes;
		FlatSet<LightComponent*> m_directional_lights;

	public:
		WorldEnvironment environment;

		u32 add_primitive(PrimitiveComponent* primitive, const Box3i& box);
		Scene& update_primitive(u32 id, const Box3i& box);
		Scene& remove_primitive(u32 id);

		u32 add_light(LightComponent* light, const Box3i& box);
		Scene& update_light(u32 id, const Box3i& box);
		Scene& remove_light(u32 id);

		u32 add_post_process(PostProcessComponent* post_process, const Box3i& box);
		Scene& update_post_process(u32 id, const Box3i& box);
		Scene& remove_post_process(u32 id);

		inline const Octree& primitive_octree() const { return m_primitive_octree; }
		inline const Octree& light_octree() const { return m_light_octree; };
		inline const Octree& post_process_octree() const { return m_post_process_octree; }

		FrameVector<PrimitiveComponent*> collect_visible_primitives(const Frustum& frustum);
		FrameVector<LightComponent*> collect_visible_lights(const Frustum& frustum);
		FrameVector<PostProcessComponent*> collect_post_processes(const Vector3f& location);
		const Box3i& bounds() const;
	};
}// namespace Trinex
