#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/scene.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	Renderer::Renderer(Scene* scene, const SceneView& view, ViewMode mode) : m_scene(scene), m_view(view), m_view_mode(mode)
	{
		m_graph = new (FrameAllocator<RenderGraph::Graph>::allocate(1)) RenderGraph::Graph();
	}

	RHISurfaceFormat Renderer::format_of(SurfaceType type)
	{
		switch (type)
		{
			case SurfaceType::SceneColor:
				return Settings::Rendering::enable_hdr ? RHISurfaceFormat::RGBA16F : RHISurfaceFormat::RGBA8;
			case SurfaceType::SceneDepth: return RHISurfaceFormat::Depth;
			case SurfaceType::BaseColor: return RHISurfaceFormat::RGBA8;
			case SurfaceType::Normal: return RHISurfaceFormat::RGBA16F;
			case SurfaceType::Emissive: return RHISurfaceFormat::RGBA8;
			case SurfaceType::MSRA: return RHISurfaceFormat::RGBA8;
			default: return RHISurfaceFormat::RGBA8;
		}
	}

	Renderer* Renderer::static_create_renderer(Scene* scene, const SceneView& view, ViewMode mode)
	{
		return new (FrameAllocator<DeferredRenderer>::allocate(1)) DeferredRenderer(scene, view, mode);
	}

	Renderer& Renderer::render()
	{
		rhi->viewport(m_view.viewport());
		rhi->scissor(m_view.scissor());
		m_graph->execute();

		return *this;
	}

	Renderer& Renderer::render_primitive(RenderPass* pass, PrimitiveComponent* primitive, const MaterialBindings* bindings)
	{
		auto proxy = primitive->proxy();

		if (!proxy->has_render_data())
			return *this;

		const uint_t lod      = scene_view().camera_view().compute_lod(proxy->world_transform().location(), proxy->lods());
		const uint_t surfaces = proxy->surfaces(lod);

		for (uint_t surface_index = 0; surface_index < surfaces; ++surface_index)
		{
			const MeshSurface* surface = proxy->surface(surface_index, lod);

			if (surface == nullptr)
				continue;

			MaterialInterface* material_interface = proxy->material(surface->material_index, lod);

			if (material_interface == nullptr)
				continue;

			Material* material         = material_interface->material();
			GraphicsPipeline* pipeline = material->pipeline(pass);

			if (!pipeline)
				continue;

			VertexShader* shader = pipeline->vertex_shader();
			RendererContext ctx(this, pass, proxy->world_transform().matrix());

			if (!material_interface->apply(ctx, bindings))
				continue;

			for (Index i = 0, count = shader->attributes.size(); i < count; ++i)
			{
				auto& attribute          = shader->attributes[i];
				VertexBufferBase* buffer = proxy->find_vertex_buffer(attribute.semantic, attribute.semantic_index, lod);

				if (buffer)
				{
					buffer->rhi_bind(attribute.stream_index, 0);
				}
			}

			if (auto index_buffer = proxy->find_index_buffer(lod))
			{
				index_buffer->rhi_bind();
				rhi->draw_indexed(surface->vertices_count, surface->first_index, surface->base_vertex_index);
			}
			else
			{
				rhi->draw(surface->vertices_count, surface->base_vertex_index);
			}
		}
		return *this;
	}

	RHI_Texture* Renderer::surface(SurfaceType type)
	{
		if (m_surfaces[type] == nullptr)
		{
			static const char* clear_pass_names[] = {"Clear SceneColor", "Clear SceneDepth", "Clear BaseColor",
			                                         "Clear Normal",     "Clear Emissive",   "Clear MSRA"};

			auto pool           = RHISurfacePool::global_instance();
			RHI_Texture* target = pool->request_transient_surface(format_of(type), m_view.viewport().size);
			m_surfaces[type]    = target;

			auto& pass = m_graph->add_pass(RenderGraph::Pass::Graphics, clear_pass_names[type])
			                     .add_resource(target, RHIAccess::CopyDst);

			if (type == SceneDepth)
				pass.add_func([target]() { target->as_dsv()->clear(1.f, 0); });
			else
				pass.add_func([target]() { target->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 1.f)); });
		}

		return m_surfaces[type];
	}

	RHI_Buffer* Renderer::globals_uniform_buffer()
	{
		if (m_globals == nullptr)
		{
			m_globals = RHIBufferPool::global_instance()->request_transient_buffer(sizeof(GlobalShaderParameters),
			                                                                       RHIBufferCreateFlags::UniformBuffer);

			GlobalShaderParameters params;
			params.update(&m_view);
			rhi->barrier(m_globals, RHIAccess::CopyDst);
			rhi->update_buffer(m_globals, 0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
			rhi->barrier(m_globals, RHIAccess::UniformBuffer);
		}
		return m_globals;
	}
}// namespace Engine
