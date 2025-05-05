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
		trinex_refl_prop(static_class_instance(), This, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_class_instance());
		r.property("StaticMesh@ mesh", &This::mesh);
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

		StaticMesh* mesh = component->mesh;

		if (mesh->lods.empty())
			return *this;

		auto& camera_view = scene_view().camera_view();
		auto& lod = mesh->lods[camera_view.compute_lod(component->proxy()->world_transform().location(), mesh->lods.size())];

		for (auto& surface_info : mesh->materials)
		{
			if (static_cast<Index>(surface_info.surface_index) > lod.surfaces.size())
			{
				continue;
			}

			MaterialInterface* material_interface = component->material(surface_info.surface_index);

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

				auto& surface = lod.surfaces[surface_info.surface_index];

				pass->predraw(component, surface_info.material, pipeline);

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
		if (mesh)
			renderer->render_component(this);
		return *this;
	}

	StaticMeshComponent& StaticMeshComponent::update_bounding_box()
	{
		if (mesh)
		{
			m_bounding_box = mesh->bounds.apply_transform(world_transform().matrix());
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

		if (mesh && index < mesh->materials.size())
			return mesh->materials[index].material;

		return nullptr;
	}
}// namespace Engine
