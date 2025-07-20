#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/material.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(MeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_prop(static_reflection(), This, m_material_overrides)->tooltip("Material overrides of this component");
	}

	MaterialInterface* MeshComponent::Proxy::material(size_t index) const
	{
		if (index < m_material_overrides.size())
			return m_material_overrides[index];
		return nullptr;
	}

	MeshComponent::Proxy& MeshComponent::Proxy::material(MaterialInterface* material, size_t index)
	{
		if (index >= m_material_overrides.size())
		{
			m_material_overrides.resize(index + 1, nullptr);
			m_material_overrides[index] = material;
		}
		else if (index == m_material_overrides.size() - 1 && material == nullptr)
		{
			m_material_overrides.pop_back();
		}
		return *this;
	}

	MaterialInterface* MeshComponent::material(size_t index) const
	{
		if (index < m_material_overrides.size())
			return m_material_overrides[index];
		return nullptr;
	}

	MeshComponent& MeshComponent::material(MaterialInterface* material, size_t index)
	{
		if (index >= m_material_overrides.size())
		{
			if (material)
			{
				m_material_overrides.resize(index + 1, nullptr);
				m_material_overrides[index] = material;

				render_thread()->call(
				        [self = proxy(), material = Pointer(material), index]() { self->material(material, index); });
			}
		}
		else if (index == m_material_overrides.size() - 1 && material == nullptr)
		{
			m_material_overrides.pop_back();
			render_thread()->call([self = proxy(), index]() { self->material(nullptr, index); });
		}
		return *this;
	}
}// namespace Engine
