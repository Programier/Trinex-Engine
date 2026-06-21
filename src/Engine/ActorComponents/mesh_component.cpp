#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Trinex
{
	trinex_implement_engine_class(MeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_prop(m_material_overrides)->tooltip("Material overrides of this component");
	}

	MaterialInterface* MeshComponent::material(usize index) const
	{
		if (index < m_material_overrides.size())
			return m_material_overrides[index];
		return nullptr;
	}

	MeshComponent& MeshComponent::material(MaterialInterface* material, usize index)
	{
		if (index >= m_material_overrides.size())
		{
			if (material)
			{
				m_material_overrides.resize(index + 1, nullptr);
				m_material_overrides[index] = material;
			}
		}
		else if (index == m_material_overrides.size() - 1 && material == nullptr)
		{
			m_material_overrides.pop_back();
		}
		return *this;
	}
}// namespace Trinex
