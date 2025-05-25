#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
	Renderer::Renderer(Scene* scene, const SceneView& view, ViewMode mode)
	    : m_view(view), m_scene(scene), m_view_mode(mode),
	      m_visible_primitives(scene->collect_visible_primitives(view.camera_view())),
	      m_visible_lights(scene->collect_visible_lights(view.camera_view()))
	{
		m_custom_passes.reserve(8);
		m_globals = RHIBufferPool::global_instance()->request_transient_buffer(sizeof(GlobalShaderParameters),
		                                                                       BufferCreateFlags::UniformBuffer);

		GlobalShaderParameters params;
		params.update(&view);
		rhi->barrier(m_globals, RHIAccess::CopyDst);
		rhi->update_buffer(m_globals, 0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
		rhi->barrier(m_globals, RHIAccess::UniformBuffer);

		auto surface_manager = RHISurfacePool::global_instance();

		for (uint32_t index = 0; index != Surface::LastSurface; ++index)
		{
			Surface type      = static_cast<Surface>(index);
			m_surfaces[index] = surface_manager->request_transient_surface(format_of(type), m_view.viewport().size);
		}
	}

	SurfaceFormat Renderer::format_of(Surface type)
	{
		switch (type)
		{
			case Surface::SceneColor: return Settings::Rendering::enable_hdr ? SurfaceFormat::RGBA16F : SurfaceFormat::RGBA8;
			case Surface::SceneDepth: return SurfaceFormat::Depth;
			case Surface::BaseColor: return SurfaceFormat::RGBA8;
			case Surface::Normal: return SurfaceFormat::RGBA16F;
			case Surface::Emissive: return SurfaceFormat::RGBA8;
			case Surface::MSRA: return SurfaceFormat::RGBA8;
			default: return SurfaceFormat::RGBA8;
		}
	}

	Renderer* Renderer::static_create_renderer(Scene* scene, const SceneView& view, ViewMode mode)
	{
		return new (FrameAllocator<Renderer>::allocate(1)) DeferredRenderer(scene, view, mode);
	}

	Renderer& Renderer::register_clear_passes(RenderGraph::Graph& graph)
	{
		graph.add_pass(RenderGraph::Pass::Graphics, "Clear Scene Color")
		        .add_resource(m_surfaces[SceneColor], RHIAccess::CopyDst)
		        .add_func([this]() { m_surfaces[SceneColor]->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 1.f)); });

		graph.add_pass(RenderGraph::Pass::Graphics, "Clear Scene Depth")
		        .add_resource(m_surfaces[SceneDepth], RHIAccess::CopyDst)
		        .add_func([this]() { m_surfaces[SceneDepth]->as_dsv()->clear(1.f, 0); });

		graph.add_pass(RenderGraph::Pass::Graphics, "Clear Base Color")
		        .add_resource(m_surfaces[BaseColor], RHIAccess::CopyDst)
		        .add_func([this]() { m_surfaces[BaseColor]->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 1.f)); });

		graph.add_pass(RenderGraph::Pass::Graphics, "Clear Normal")
		        .add_resource(m_surfaces[Normal], RHIAccess::CopyDst)
		        .add_func([this]() { m_surfaces[Normal]->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 1.f)); });

		graph.add_pass(RenderGraph::Pass::Graphics, "Clear Emissive")
		        .add_resource(m_surfaces[Emissive], RHIAccess::CopyDst)
		        .add_func([this]() { m_surfaces[Emissive]->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 1.f)); });

		graph.add_pass(RenderGraph::Pass::Graphics, "Clear MSRA")
		        .add_resource(m_surfaces[MSRA], RHIAccess::CopyDst)
		        .add_func([this]() { m_surfaces[MSRA]->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 1.f)); });
		return *this;
	}

	Renderer& Renderer::register_batched_primitives(RenderGraph::Graph& graph)
	{
		graph.add_pass(RenderGraph::Pass::Graphics, "Batched Primitives")
		        .add_resource(m_surfaces[SceneColor], RHIAccess::RTV)
		        .add_resource(m_surfaces[SceneDepth], RHIAccess::DSV)
		        .add_func([this]() {
			        rhi->bind_render_target1(scene_color_target()->as_rtv(), scene_depth_target()->as_dsv());
			        lines.flush(this);
		        });

		return *this;
	}

	Renderer& Renderer::render()
	{
		RenderGraph::Graph graph;
		graph.add_output(m_surfaces[SceneColor]);

		register_clear_passes(graph);
		render(graph);

		for (auto custom_pass : m_custom_passes)
		{
			custom_pass->execute(this, graph);
			custom_pass->~CustomPass();
		}

		register_batched_primitives(graph);

		rhi->viewport(m_view.viewport());
		rhi->scissor(m_view.scissor());

		graph.execute();

		return *this;
	}

	Renderer& Renderer::render_primitive(RenderPass* pass, PrimitiveComponent* primitive)
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

			if (!material_interface->apply(ctx))
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

	Renderer& Renderer::render_visible_primitives(RenderPass* pass)
	{
		const FrameVector<PrimitiveComponent*>& primitives = visible_primitives();

		for (PrimitiveComponent* primitive : primitives)
		{
			render_primitive(pass, primitive);
		}

		return *this;
	}
}// namespace Engine
