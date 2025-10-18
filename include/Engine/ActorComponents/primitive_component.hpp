#pragma once
#include <Core/math/box.hpp>
#include <Engine/ActorComponents/scene_component.hpp>

namespace Engine
{
	class PrimitiveComponent;
	class MaterialInterface;
	class MeshReference;
	struct MeshSurface;
	class VertexBufferBase;
	class IndexBuffer;

	struct PrimitiveRenderingContext;

	class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
	{
		trinex_declare_class(PrimitiveComponent, SceneComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		protected:
			Box3f m_bounds;

		public:
			virtual Proxy& render(PrimitiveRenderingContext* context);

			inline const Box3f& bounding_box() const { return m_bounds; }
			friend class PrimitiveComponent;
		};

	protected:
		bool m_is_visible;
		Box3f m_bounding_box;

		void submit_bounds_to_render_thread();

	public:
		PrimitiveComponent();
		bool is_visible() const;
		PrimitiveComponent& is_visible(bool visible);
		const Box3f& bounding_box() const;

		PrimitiveComponent& start_play() override;
		PrimitiveComponent& stop_play() override;
		PrimitiveComponent& on_transform_changed() override;

		virtual size_t materials_count() const                                 = 0;
		virtual MaterialInterface* material(size_t index) const                = 0;
		virtual PrimitiveComponent& material(MaterialInterface*, size_t index) = 0;

		virtual PrimitiveComponent& update_bounding_box();
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
		~PrimitiveComponent();
	};
}// namespace Engine
