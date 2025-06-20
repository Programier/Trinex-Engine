#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <Core/name.hpp>
#include <Core/pointer.hpp>
#include <Engine/enviroment.hpp>
#include <Engine/octree.hpp>

namespace Engine
{
	class PrimitiveComponent;
	class LightComponent;
	class SceneComponent;
	class RenderViewport;
	class RenderTargetBase;
	struct Frustum;

	class ENGINE_EXPORT Scene final
	{
	public:
		using PrimitiveOctree = Octree<PrimitiveComponent*>;
		using LightOctree     = Octree<LightComponent*>;

	private:
		PrimitiveOctree m_octree_render_thread;
		PrimitiveOctree m_octree;
		LightOctree m_light_octree_render_thread;
		LightOctree m_light_octree;
		Pointer<SceneComponent> m_root_component;

	public:
		WorldEnvironment environment;

		Scene();
		Scene& add_primitive(PrimitiveComponent* primitive);
		Scene& remove_primitive(PrimitiveComponent* primitive);
		Scene& update_primitive_transform(PrimitiveComponent* primitive);
		Scene& update_light_transform(LightComponent* light);
		Scene& add_light(LightComponent* light);
		Scene& remove_light(LightComponent* light);
		SceneComponent* root_component() const;
		const PrimitiveOctree& primitive_octree() const;
		const LightOctree& light_octree() const;

		FrameVector<PrimitiveComponent*> collect_visible_primitives(const Frustum& frustum);
		FrameVector<LightComponent*> collect_visible_lights(const Frustum& frustum);
		~Scene();
	};
}// namespace Engine
