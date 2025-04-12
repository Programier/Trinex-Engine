#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>

namespace Engine
{
	class MaterialInterface;

	class ENGINE_EXPORT MeshComponent : public PrimitiveComponent
	{
		trinex_declare_class(MeshComponent, PrimitiveComponent);

	public:
		Vector<MaterialInterface*> material_overrides;
	};
}// namespace Engine
