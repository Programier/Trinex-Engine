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
		trinex_class(PrimitiveComponent, SceneComponent);

	protected:
		bool m_is_visible;
		Box3f m_bounding_box;

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
		virtual PrimitiveComponent& render(PrimitiveRenderingContext* context);

		virtual PrimitiveComponent& update_bounding_box();
		~PrimitiveComponent();
	};
}// namespace Engine
