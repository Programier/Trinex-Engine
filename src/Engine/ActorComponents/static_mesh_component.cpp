#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	trinex_implement_engine_class(StaticMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_virtual_prop(static_class_instance(), mesh, mesh, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_class_instance());
		r.method("StaticMesh@ mesh() const final", method_of<StaticMesh*>(&This::mesh));
		r.method("StaticMeshComponent@ mesh(StaticMesh@ mesh) final", method_of<StaticMeshComponent&, StaticMesh*>(&This::mesh));
	}

	size_t StaticMeshComponent::Proxy::lods() const
	{
		return m_mesh->lods.size();
	}

	size_t StaticMeshComponent::Proxy::materials_count(size_t lod) const
	{
		return m_mesh->materials.size();
	}

	size_t StaticMeshComponent::Proxy::surfaces(size_t lod) const
	{
		return m_mesh->lods[lod].surfaces.size();
	}

	const MeshSurface* StaticMeshComponent::Proxy::surface(size_t index, size_t lod) const
	{
		return &m_mesh->lods[lod].surfaces[index];
	}

	MaterialInterface* StaticMeshComponent::Proxy::material(size_t index, size_t lod) const
	{
		return m_mesh ? m_mesh->materials[index] : nullptr;
	}

	VertexBufferBase* StaticMeshComponent::Proxy::find_vertex_buffer(VertexBufferSemantic semantic, Index index, size_t lod)
	{
		return m_mesh->lods[lod].find_vertex_buffer(semantic, index);
	}

	IndexBuffer* StaticMeshComponent::Proxy::find_index_buffer(size_t lod)
	{
		auto& buffer = m_mesh->lods[lod].indices;
		return buffer.size() == 0 ? nullptr : &buffer;
	}

	StaticMeshComponent& StaticMeshComponent::submit_new_mesh()
	{
		render_thread()->call([proxy = proxy(), mesh = m_mesh]() { proxy->m_mesh = mesh; });
		return *this;
	}

	StaticMeshComponent::Proxy* StaticMeshComponent::create_proxy()
	{
		return new Proxy();
	}

	SceneRenderer& SceneRenderer::render_component(StaticMeshComponent* component)
	{
		render_base_component(component);

		if (!(scene_view().show_flags() & ShowFlags::StaticMesh))
			return *this;

		StaticMesh* mesh = component->mesh();

		if (mesh->lods.empty())
			return *this;

		auto& camera_view = scene_view().camera_view();
		auto& lod = mesh->lods[camera_view.compute_lod(component->proxy()->world_transform().location(), mesh->lods.size())];

		for (auto& surface : lod.surfaces)
		{
			if (static_cast<Index>(surface.material_index) > mesh->materials.size())
			{
				continue;
			}

			MaterialInterface* material_interface = component->material(surface.material_index);

			if (material_interface == nullptr)
				continue;

			Material* material = material_interface->material();

			for (auto pass = first_pass(); pass; pass = pass->next())
			{
				GraphicsPipeline* pipeline = Object::instance_cast<GraphicsPipeline>(material->pipeline(pass->info()));

				if (pipeline == nullptr)
				{
					// Pass is not supported by this material
					continue;
				}

				VertexShader* shader = pipeline->vertex_shader();
				pass->bind_material(material_interface, component);

				for (Index i = 0, count = shader->attributes.size(); i < count; ++i)
				{
					auto& attribute          = shader->attributes[i];
					VertexBufferBase* buffer = lod.find_vertex_buffer(attribute.semantic, attribute.semantic_index);

					if (buffer)
					{
						pass->bind_vertex_buffer(buffer, attribute.stream_index, 0);
					}
				}

				pass->predraw(component, material_interface, pipeline);

				if (lod.indices.size() > 0)
				{
					pass->bind_index_buffer(&lod.indices, 0);
					pass->draw_indexed(surface.vertices_count, surface.first_index, surface.base_vertex_index);
				}
				else
				{
					pass->draw(surface.vertices_count, surface.base_vertex_index);
				}
			}
		}

		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::render(class SceneRenderer* renderer)
	{
		if (mesh())
			renderer->render_component(this);
		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::update_bounding_box()
	{
		if (mesh())
		{
			m_bounding_box = mesh()->bounds.apply_transform(world_transform().matrix());
			submit_bounds_to_render_thread();
		}
		else
		{
			Super::update_bounding_box();
		}
		return *this;
	}

	MaterialInterface* StaticMeshComponent::material(size_t index) const
	{
		if (index < material_overrides.size())
		{
			if (auto material = material_overrides[index])
				return material;
		}

		if (mesh() && index < mesh()->materials.size())
			return mesh()->materials[index];

		return nullptr;
	}
}// namespace Engine
