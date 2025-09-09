#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(MeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_prop(m_material_overrides)->tooltip("Material overrides of this component");
	}

	MaterialInterface* MeshComponent::Proxy::material(size_t index) const
	{
		if (index < m_material_overrides.size())
			return m_material_overrides[index];
		return nullptr;
	}

	MeshComponent::Proxy& MeshComponent::Proxy::render(Renderer* renderer, RenderPass* pass, const MaterialBindings* bindings)
	{
		const auto& camera = renderer->scene_view().camera_view();

		const uint_t lod      = camera.compute_lod(world_transform().location(), lods_count());
		const uint_t surfaces = surfaces_count(lod);

		for (uint_t surface_index = 0; surface_index < surfaces; ++surface_index)
		{
			const MeshSurface* surface_data = surface(surface_index, lod);

			if (surface_data == nullptr)
				continue;

			MaterialInterface* material_interface = material(surface_data->material_index);

			if (material_interface == nullptr)
				continue;

			Material* material         = material_interface->material();
			GraphicsPipeline* pipeline = material->pipeline(pass);

			if (!pipeline)
				continue;

			RendererContext ctx(renderer, pass, world_transform().matrix());

			if (!material_interface->apply(ctx, bindings))
				continue;

			for (Index i = 0, count = pipeline->vertex_attributes.size(); i < count; ++i)
			{
				auto& attribute          = pipeline->vertex_attributes[i];
				VertexBufferBase* buffer = find_vertex_buffer(attribute.semantic, attribute.semantic_index, lod);

				if (buffer)
				{
					buffer->rhi_bind(attribute.stream_index, 0);
				}
			}

			if (auto index_buffer = find_index_buffer(lod))
			{
				index_buffer->rhi_bind();
				rhi->draw_indexed(surface_data->vertices_count, surface_data->first_index, surface_data->base_vertex_index);
			}
			else
			{
				rhi->draw(surface_data->vertices_count, surface_data->base_vertex_index);
			}
		}
		return *this;
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
