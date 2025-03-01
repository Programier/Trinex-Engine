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
	implement_engine_class(StaticMeshComponent, Refl::Class::IsScriptable)
	{
		trinex_refl_prop(static_class_instance(), This, mesh)->tooltip("Mesh object of this component");

		auto r = ScriptClassRegistrar::existing_class(static_class_instance());
		r.property("StaticMesh@ mesh", &This::mesh);
	}

	StaticMeshComponent& StaticMeshComponent::update(float dt)
	{
		Super::update(dt);
		return *this;
	}

	implement_empty_rendering_methods_for(StaticMeshComponent);

	ColorSceneRenderer& ColorSceneRenderer::render_component(StaticMeshComponent* component)
	{
		render_base_component(component);

		if (!(scene_view().show_flags() & ShowFlags::StaticMesh))
			return *this;

		StaticMesh* mesh   = component->mesh;
		auto& camera_view  = scene_view().camera_view();
		float inv_distance = 1.f / glm::min(glm::distance(component->proxy()->world_transform().location(), camera_view.location),
		                                    camera_view.far_clip_plane);
		auto& lods         = mesh->lods;
		Index lod_index    = glm::min<Index>(static_cast<Index>(static_cast<float>(lods.size()) * inv_distance), lods.size() - 1);
		auto& lod          = lods[lod_index];

		for (auto& material : mesh->materials)
		{
			if (material.material == nullptr || static_cast<Index>(material.surface_index) > lod.surfaces.size())
			{
				continue;
			}

			auto pass = geometry_pass();
			pass->bind_material(material.material, component);

			VertexShader* shader = Object::instance_cast<VertexShader>(
					material.material->material()->pipeline(nullptr)->shader(ShaderType::Vertex));

			for (Index i = 0, count = shader->attributes.size(); i < count; ++i)
			{
				auto& attribute      = shader->attributes[i];
				VertexBuffer* buffer = lod.find_vertex_buffer(attribute.semantic, attribute.semantic_index);

				if (buffer)
				{
					pass->bind_vertex_buffer(buffer, attribute.stream_index, 0);
				}
			}

			auto& surface = lod.surfaces[material.surface_index];

			if (lod.indices->size() > 0)
			{
				pass->bind_index_buffer(lod.indices, 0);
				pass->draw_indexed(surface.vertices_count, surface.first_index, surface.base_vertex_index);
			}
			else
			{
				pass->draw(surface.vertices_count, surface.base_vertex_index);
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
}// namespace Engine
