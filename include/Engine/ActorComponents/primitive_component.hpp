#pragma once
#include <Core/math/box.hpp>
#include <Engine/ActorComponents/scene_component.hpp>

namespace Trinex
{
	class PrimitiveComponent;
	class MaterialInterface;
	class MeshReference;
	struct MeshSurface;
	class VertexBufferBase;
	class IndexBuffer;

	struct PrimitiveRenderingContext;

	class trinex_api PrimitiveComponent : public SceneComponent
	{
		trinex_class(PrimitiveComponent, SceneComponent);

	public:
		PrimitiveComponent();
		~PrimitiveComponent();
	};
} // namespace Trinex
