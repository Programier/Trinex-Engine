#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
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
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	Renderer::Renderer(Scene* scene, const SceneView& view, ViewMode mode) : m_scene(scene), m_view(view), m_view_mode(mode)
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

	Renderer& Renderer::render_primitives(PrimitiveComponent* const* primitives, usize count, RHIContext* ctx, RenderPass* pass,
	                                      RHIContextInheritanceInfo* inherit, MaterialBindings* bindings)
	{
		trinex_profile_cpu_n("Renderer::render_primitives");

		static constexpr u32 chunk = 64;

		struct Worker {
			RHIContext* context = nullptr;
			usize objects;
		};

		TaskGraph* graph             = TaskGraph::instance();
		RHIContextPool* context_pool = RHIContextPool::global_instance();

		//const u32 worker_count     = Math::min<u32>(graph->workers() + 1, (count + chunk - 1) / chunk);
		const u32 worker_count = 1;

		StackByteAllocator::Mark mark;
		Worker* workers = StackAllocator<Worker>::allocate(worker_count);

		for (u32 i = 0; i < worker_count; ++i)
		{
			RHIContext* secondary = context_pool->request_context(RHIContextFlags::Secondary);
			workers[i]            = {secondary, 0};
		}

		auto callback = [this, primitives, pass, workers, bindings, inherit](u32 idx) {
			Worker& worker = workers[Task::worker_index()];

			if (++worker.objects == 1)
			{
				worker.context->begin(inherit);
			}

			PrimitiveComponent* primitive = primitives[idx];
			Matrix4f matrix               = primitive->world_transform().matrix();
			PrimitiveRenderingContext context(this, worker.context, pass, &matrix, bindings);
			primitive->render(&context);
		};

		pass->begin(this, ctx);
		{
			graph->for_each(count, callback, chunk, worker_count);

			for (u32 i = 0; i < worker_count; ++i)
			{
				Worker& worker = workers[i];

				if (worker.objects)
				{
					RHICommandHandle* handle = worker.context->end();
					ctx->execute(handle);
					handle->release();
				}

				context_pool->return_context(worker.context);
			}
		}
		pass->end(this, ctx);

		return *this;
	}

	Renderer& Renderer::render(RHIContext* ctx)
	{
		while (m_child_renderer)
		{
			m_child_renderer->renderer->render(ctx);
			m_child_renderer = m_child_renderer->next;
		}

		ctx->viewport(m_view.viewport());
		ctx->scissor(m_view.scissor());

		RHIBuffer* view = globals_uniform_buffer();

		GlobalShaderParameters params;
		params.update(&m_view, m_view.view_size());
		ctx->barrier(view, RHIAccess::TransferDst);
		ctx->update_buffer(view, 0, sizeof(GlobalShaderParameters), reinterpret_cast<const u8*>(&params));
		ctx->barrier(view, RHIAccess::UniformBuffer);

		m_graph->execute(ctx);

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

			auto pool          = RHITexturePool::global_instance();
			RHITexture* target = pool->request_transient_surface(surface_format_of(type), m_view.view_size());

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
			m_globals = RHIBufferPool::global_instance()->request_transient_buffer(sizeof(GlobalShaderParameters),
			                                                                       RHIBufferCreateFlags::UniformBuffer);
		}
		return m_globals;
	}
}// namespace Trinex
