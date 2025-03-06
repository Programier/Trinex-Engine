#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
	class PrimitiveComponent;

	class ENGINE_EXPORT PrimitiveComponentProxy : public SceneComponentProxy
	{
	protected:
		AABB_3Df m_bounds;

	public:
		PrimitiveComponentProxy& bounding_box(const AABB_3Df& bounds);
		const AABB_3Df& bounding_box() const;
		friend class PrimitiveComponent;
	};

	class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
	{
		trinex_declare_class(PrimitiveComponent, SceneComponent);

	protected:
		bool m_is_visible;
		AABB_3Df m_bounding_box;

		void submit_bounds_to_render_thread();

	public:
		PrimitiveComponent();
		bool is_visible() const;
		PrimitiveComponent& is_visible(bool visible);
		const AABB_3Df& bounding_box() const;

		PrimitiveComponent& start_play() override;
		PrimitiveComponent& stop_play() override;
		PrimitiveComponent& on_transform_changed() override;
		ActorComponentProxy* create_proxy() override;

		virtual PrimitiveComponent& render(class SceneRenderer* renderer);
		virtual PrimitiveComponent& update_bounding_box();

		PrimitiveComponentProxy* proxy() const;
		~PrimitiveComponent();
	};
}// namespace Engine
