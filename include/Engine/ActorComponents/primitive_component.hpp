#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
	class PrimitiveComponent;
	class MaterialInterface;
	class MeshReference;
	struct MeshSurface;
	class VertexBufferBase;
	class IndexBuffer;

	struct RHIVertexBufferSemantic;

	class ENGINE_EXPORT PrimitiveComponent : public SceneComponent
	{
		trinex_declare_class(PrimitiveComponent, SceneComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		protected:
			AABB_3Df m_bounds;

		public:
			virtual bool has_render_data() const                                                                            = 0;
			virtual size_t lods() const                                                                                     = 0;
			virtual size_t materials_count(size_t lod = 0) const                                                            = 0;
			virtual size_t surfaces(size_t lod = 0) const                                                                   = 0;
			virtual const MeshSurface* surface(size_t index, size_t lod = 0) const                                          = 0;
			virtual MaterialInterface* material(size_t index, size_t lod = 0) const                                         = 0;
			virtual VertexBufferBase* find_vertex_buffer(RHIVertexBufferSemantic semantic, Index index = 0, size_t lod = 0) = 0;
			virtual IndexBuffer* find_index_buffer(size_t lod = 0)                                                          = 0;

			inline const AABB_3Df& bounding_box() const { return m_bounds; }
			friend class PrimitiveComponent;
		};

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

		virtual PrimitiveComponent& update_bounding_box();

		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
		~PrimitiveComponent();
	};
}// namespace Engine
