#pragma once
#include <Engine/ActorComponents/primitive_component.hpp>

namespace Engine
{
	class MaterialInterface;

	class ENGINE_EXPORT MeshComponent : public PrimitiveComponent
	{
		trinex_declare_class(MeshComponent, PrimitiveComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		private:
			Vector<MaterialInterface*> m_material_overrides;

		private:
			Proxy& material(MaterialInterface*, size_t index);

		public:
			MaterialInterface* material(size_t index) const override;

			friend class MeshComponent;
		};

	private:
		Vector<MaterialInterface*> m_material_overrides;

	public:
		MaterialInterface* material(size_t index) const override;
		MeshComponent& material(MaterialInterface*, size_t index) override;

		inline size_t overrided_material_count() const { return m_material_overrides.size(); }
		inline MaterialInterface* overrided_material(size_t index) const { return m_material_overrides[index]; }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
	};
}// namespace Engine
