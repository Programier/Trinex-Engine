#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/Render/scene.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	Renderer::Renderer(const SceneView& view, ViewMode mode) : m_view(view), m_view_mode(mode)
	{
		m_graph = new (FrameAllocator<RenderGraph::Graph>::allocate(1)) RenderGraph::Graph();
	}

	RHISurfaceFormat Renderer::surface_format_of(SurfaceType type)
	{
		switch (type)
		{
			case SurfaceType::SceneColorHDR: return RHISurfaceFormat::RGBA16F;
			case SurfaceType::SceneColorLDR: return RHISurfaceFormat::RGBA8;
			case SurfaceType::SceneDepth: return RHISurfaceFormat::D24S8;
			case SurfaceType::BaseColor: return RHISurfaceFormat::RGBA8;
			case SurfaceType::Normal: return RHISurfaceFormat::RGB10A2;
			case SurfaceType::MSRA: return RHISurfaceFormat::RGBA8;
			case SurfaceType::Velocity: return RHISurfaceFormat::RG16F;
			default: return RHISurfaceFormat::RGBA8;
		}
	}

	RHITextureFlags Renderer::surface_flags_of(SurfaceType type)
	{
		using F = RHITextureFlags;

		switch (type)
		{
			case SurfaceType::SceneColorHDR: return RHITextureFlags::RWColorAttachment;
			case SurfaceType::SceneColorLDR: return RHITextureFlags::ColorAttachment;
			case SurfaceType::SceneDepth: return RHITextureFlags::DepthStencilAttachment;
			case SurfaceType::BaseColor: return RHITextureFlags::ColorAttachment;
			case SurfaceType::Normal: return RHITextureFlags::ColorAttachment;
			case SurfaceType::MSRA: return RHITextureFlags::ColorAttachment;
			case SurfaceType::Velocity: return RHITextureFlags::ColorAttachment;
			default: return F::Undefined;
		}
	}

	const char* Renderer::surface_name_of(SurfaceType type)
	{
		switch (type)
		{
			case SurfaceType::SceneColorHDR: return "SceneColorHDR";
			case SurfaceType::SceneColorLDR: return "SceneColorLDR";
			case SurfaceType::SceneDepth: return "SceneDepth";
			case SurfaceType::BaseColor: return "BaseColor";
			case SurfaceType::Normal: return "Normal";
			case SurfaceType::MSRA: return "MSRA";
			case SurfaceType::Velocity: return "Velocity";
			default: return "Undefined";
		}
	}

	void Renderer::sort_lights(FrameVector<LightComponent*>& visible_lights)
	{
		std::sort(visible_lights.begin(), visible_lights.end(), [](LightComponent* a, LightComponent* b) -> bool {
			auto a_type = a->light_type();
			auto b_type = b->light_type();

			if (a_type != b_type)
				return a_type < b_type;

			bool a_has_shadow = a->is_shadows_enabled();
			bool b_has_shadow = b->is_shadows_enabled();

			if (a_has_shadow != b_has_shadow)
				return !a_has_shadow;

			return false;
		});
	}

	Renderer& Renderer::render_primitives(RHIContext* ctx, RenderPass* pass)
	{
		trinex_profile_cpu_n("Renderer::render_primitives");

		RenderScene* render_scene = scene();

		for (auto& chunk : render_scene->chunks())
		{
			Pipeline* pipeline = chunk.material->pipeline(pass);

			if (pipeline == nullptr)
				continue;

			// Temporary hack to render all objects

			ctx->bind_pipeline(pipeline->rhi_pipeline());
			ctx->bind_uniform_buffer(globals_uniform_buffer(), 0);
			u32* indices = render_scene->map<u32>(chunk.address);

			for (u32 i = 0; i < chunk.count; ++i)
			{
				u32 address     = indices[i];
				auto& primitive = render_scene->primitive(address);

				ctx->draw(RHITopology::TriangleList, primitive.vertices_count, 0, 1, address);
			}
		}

		return *this;
	}

	Renderer& Renderer::render(RHIContext* ctx)
	{
		while (m_child_renderer)
		{
			m_child_renderer->renderer->render(ctx);
			m_child_renderer = m_child_renderer->next;
		}

		scene()->flush(ctx);

		ctx->viewport(m_view.viewport());
		ctx->scissor(m_view.scissor());

		RHIBuffer* view = globals_uniform_buffer();

		GlobalShaderParameters params;
		params.update(&m_view);
		ctx->barrier(view, RHIAccess::TransferDst);
		ctx->update(view, &params, {.size = sizeof(GlobalShaderParameters)});
		ctx->barrier(view, RHIAccess::UniformBuffer);

		m_graph->execute(ctx);

		scene_view().flush(this);
		return *this;
	}

	Renderer& Renderer::reset(const SceneView& view)
	{
		m_view    = view;
		m_globals = nullptr;
		return *this;
	}

	Renderer& Renderer::add_child_renderer(Renderer* renderer)
	{
		if (renderer)
		{
			ChildRenderer* child = FrameAllocator<ChildRenderer>::allocate(1);
			child->renderer      = renderer;
			child->next          = m_child_renderer;
			m_child_renderer     = child;
		}

		return *this;
	}

	RHITexture* Renderer::surface(SurfaceType type)
	{
		return surface_clear_pass(type)->resources()[0]->as_texture();
	}

	RenderGraph::Pass* Renderer::surface_clear_pass(SurfaceType type)
	{
		if (m_surface_clears[type] == nullptr)
		{
			static const char* clear_pass_names[] = {
			        "Clear SceneColor HDR", "Clear SceneColor LDR", "Clear SceneDepth", "Clear BaseColor",
			        "Clear Normal",         "Clear MSRA",           "Clear Velocity",
			};

			auto pool               = RHITexturePool::global_instance();
			RHISurfaceFormat format = surface_format_of(type);
			RHITextureFlags flags   = surface_flags_of(type);
			RHITexture* target      = pool->acquire_transient(format, m_view.view_size(), flags);

			RenderGraph::Pass* pass = &m_graph->add_pass(clear_pass_names[type]).add_resource(target, RHIAccess::TransferDst);
			m_surface_clears[type]  = pass;

			if (type == SceneDepth)
				pass->add_func([target](RHIContext* ctx) { ctx->clear_dsv(target->as_dsv()); });
			else
				pass->add_func([target](RHIContext* ctx) { ctx->clear_rtv(target->as_rtv()); });
		}

		return m_surface_clears[type];
	}

	RHIBuffer* Renderer::globals_uniform_buffer()
	{
		if (m_globals == nullptr)
		{
			m_globals = RHIBufferPool::global_instance()->acquire_transient(sizeof(GlobalShaderParameters),
			                                                                RHIBufferFlags::UniformBuffer);
		}
		return m_globals;
	}

	RHITexture* Renderer::request_surface(RHISurfaceFormat format, float scale)
	{
		Vector2u size = Vector2u(Vector2f(m_view.view_size()) * scale);
		return RHITexturePool::global_instance()->acquire(format, size);
	}

	RHITexture* Renderer::request_transient_surface(RHISurfaceFormat format, float scale)
	{
		Vector2u size = Vector2u(Vector2f(m_view.view_size()) * scale);
		return RHITexturePool::global_instance()->acquire_transient(format, size);
	}

	Renderer& Renderer::return_surface(RHITexture* surface)
	{
		RHITexturePool::global_instance()->release(surface);
		return *this;
	}
}// namespace Trinex
