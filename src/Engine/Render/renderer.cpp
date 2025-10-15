#include <Engine/ActorComponents/light_component.hpp>
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

	RHISurfaceFormat Renderer::static_surface_format_of(SurfaceType type)
	{
		switch (type)
		{
			case SurfaceType::SceneColorHDR: return RHISurfaceFormat::RGBA16F;
			case SurfaceType::SceneColorLDR: return RHISurfaceFormat::RGBA8;
			case SurfaceType::SceneDepth: return RHISurfaceFormat::D32F;
			case SurfaceType::BaseColor: return RHISurfaceFormat::RGBA8;
			case SurfaceType::Normal: return RHISurfaceFormat::RGB10A2;
			case SurfaceType::Emissive: return RHISurfaceFormat::RGBA16F;
			case SurfaceType::MSRA: return RHISurfaceFormat::RGBA8;
			case SurfaceType::Velocity: return RHISurfaceFormat::RG16F;
			default: return RHISurfaceFormat::RGBA8;
		}
	}

	const char* Renderer::static_surface_name_of(SurfaceType type)
	{
		switch (type)
		{
			case SurfaceType::SceneColorHDR: return "SceneColorHDR";
			case SurfaceType::SceneColorLDR: return "SceneColorLDR";
			case SurfaceType::SceneDepth: return "SceneDepth";
			case SurfaceType::BaseColor: return "BaseColor";
			case SurfaceType::Normal: return "Normal";
			case SurfaceType::Emissive: return "Emissive";
			case SurfaceType::MSRA: return "MSRA";
			case SurfaceType::Velocity: return "Velocity";
			default: return "Undefined";
		}
	}

	void Renderer::static_sort_lights(FrameVector<LightComponent*>& visible_lights)
	{
		std::sort(visible_lights.begin(), visible_lights.end(), [](LightComponent* a, LightComponent* b) -> bool {
			auto a_proxy = a->proxy();
			auto b_proxy = b->proxy();

			auto a_type = a_proxy->light_type();
			auto b_type = b_proxy->light_type();

			if (a_type != b_type)
				return a_type < b_type;

			bool a_has_shadow = a_proxy->is_shadows_enabled();
			bool b_has_shadow = b_proxy->is_shadows_enabled();

			if (a_has_shadow != b_has_shadow)
				return !a_has_shadow;

			return false;
		});
	}

	Renderer& Renderer::render()
	{
		while (m_child_renderer)
		{
			m_child_renderer->renderer->render();
			m_child_renderer = m_child_renderer->next;
		}

		rhi->viewport(m_view.viewport());
		rhi->scissor(m_view.scissor());
		m_graph->execute();

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
		if (m_surfaces[type] == nullptr)
		{
			static const char* clear_pass_names[] = {
			        "Clear SceneColor HDR", "Clear SceneColor LDR", "Clear SceneDepth", "Clear BaseColor",
			        "Clear Normal",         "Clear Emissive",       "Clear MSRA",       "Clear Velocity",
			};

			auto pool          = RHITexturePool::global_instance();
			RHITexture* target = pool->request_transient_surface(static_surface_format_of(type), m_view.viewport().size);
			m_surfaces[type]   = target;

			auto& pass = m_graph->add_pass(RenderGraph::Pass::Graphics, clear_pass_names[type])
			                     .add_resource(target, RHIAccess::TransferDst);

			if (type == SceneDepth)
				pass.add_func([target]() { target->as_dsv()->clear(1.f, 0); });
			else
				pass.add_func([target]() { target->as_rtv()->clear(LinearColor(0.f, 0.f, 0.f, 0.f)); });
		}

		return m_surfaces[type];
	}

	RHIBuffer* Renderer::globals_uniform_buffer()
	{
		if (m_globals == nullptr)
		{
			m_globals = RHIBufferPool::global_instance()->request_transient_buffer(sizeof(GlobalShaderParameters),
			                                                                       RHIBufferCreateFlags::UniformBuffer);

			GlobalShaderParameters params;
			params.update(&m_view, m_view.view_size());
			rhi->barrier(m_globals, RHIAccess::TransferDst);
			rhi->update_buffer(m_globals, 0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
			rhi->barrier(m_globals, RHIAccess::UniformBuffer);
		}
		return m_globals;
	}
}// namespace Engine
