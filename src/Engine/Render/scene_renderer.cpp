#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
	SceneRenderer::SceneRenderer() : scene(nullptr)
	{}

	const GlobalShaderParameters& SceneRenderer::global_shader_parameters() const
	{
		if (m_global_shader_params.empty())
			throw EngineException("Shader parameters stack is empty!");
		return m_global_shader_params.back();
	}

	const SceneView& SceneRenderer::scene_view() const
	{
		if (m_scene_views.empty())
			throw EngineException("Scene Views stack is empty!");
		return m_scene_views.back();
	}

	SceneRenderer& SceneRenderer::push_global_shader_parameters()
	{
		const SceneView& view = scene_view();
		m_global_shader_params.emplace_back();
		m_global_shader_params.back().update(&view);
		rhi->push_global_params(m_global_shader_params.back());
		return *this;
	}

	SceneRenderer& SceneRenderer::pop_global_shader_parameters()
	{
		rhi->pop_global_params();
		m_global_shader_params.pop_back();
		return *this;
	}

	SceneRenderer& SceneRenderer::view_mode(ViewMode new_mode)
	{
		if (is_in_render_thread())
		{
			m_view_mode = new_mode;
		}
		else
		{
			call_in_render_thread([this, new_mode]() { m_view_mode = new_mode; });
		}
		return *this;
	}


	RenderPass* SceneRenderer::create_pass_internal(RenderPass* next, const Function<RenderPass*()>& alloc)
	{
		if (next && next->m_renderer != this)
		{
			return nullptr;
		}

		RenderPass* pass = alloc();
		pass->m_renderer = this;
		pass->m_next     = next;

		if (next == m_first_pass)
		{
			m_first_pass = pass;
		}

		if (next == nullptr)
		{
			if (m_last_pass)
			{
				m_last_pass->m_next = pass;
			}
			m_last_pass = pass;
		}

		return pass;
	}

	bool SceneRenderer::destroy_pass(RenderPass* pass)
	{
		if (pass->m_renderer != this)
			return false;

		if (pass == m_first_pass)
		{
			m_first_pass = pass->m_next;
		}
		else
		{
			for (auto current = m_first_pass; current->m_next; current = current->m_next)
			{
				if (current->m_next == pass)
				{
					current->m_next = pass->m_next;

					if (m_last_pass == pass)
					{
						m_last_pass = current;
					}

					break;
				}
			}
		}

		delete pass;
		return true;
	}

	RenderSurface* SceneRenderer::output_surface() const
	{
		return SceneRenderTargets::instance()->surface_of(SceneRenderTargets::SceneColorLDR);
	}

	SceneRenderer& SceneRenderer::blit(class Texture2D* texture, const Vector2D& min, const Vector2D& max)
	{
		static Name screen_texture = "screen_texture";
		static Name min_point      = "min_point";
		static Name max_point      = "max_point";

		Material* material = DefaultResources::Materials::screen;

		if (material == nullptr)
			return *this;

		auto* texture_param  = Object::instance_cast<MaterialParameters::Sampler2D>(material->find_parameter(screen_texture));
		auto min_point_param = Object::instance_cast<MaterialParameters::Float2>(material->find_parameter(min_point));
		auto max_point_param = Object::instance_cast<MaterialParameters::Float2>(material->find_parameter(max_point));

		if (!all_of(texture_param, min_point_param, max_point_param))
			return *this;

		texture_param->texture = texture;
		min_point_param->value = min;
		max_point_param->value = max;

		material->apply();
		rhi->draw(6, 0);
		return *this;
	}

	SceneRenderer& SceneRenderer::render(const SceneView& view, RenderViewport* viewport)
	{
		if (scene == nullptr)
			return *this;

		statistics.reset();

		m_global_shader_params.clear();
		m_scene_views.clear();
		m_scene_views.push_back(view);

		rhi->viewport(view.viewport());
		rhi->scissor(view.scissor());

		push_global_shader_parameters();

		for (auto pass = first_pass(); pass; pass = pass->next())
		{
			pass->clear();
		}

		scene->build_views(this);

		for (auto pass = first_pass(); pass; pass = pass->next())
		{
			if (pass->is_empty())
				continue;

#if TRINEX_DEBUG_BUILD
			rhi->push_debug_stage(pass->struct_instance()->name().c_str());
#endif
			pass->render(viewport);
			pass->on_render(viewport, pass);

#if TRINEX_DEBUG_BUILD
			rhi->pop_debug_stage();
#endif
		}

		pop_global_shader_parameters();
		m_scene_views.pop_back();

		return *this;
	}

	SceneRenderer& SceneRenderer::render_component(PrimitiveComponent* component)
	{
		return *this;
	}

	SceneRenderer& SceneRenderer::render_component(LightComponent* component)
	{
		return *this;
	}

	SceneRenderer::~SceneRenderer()
	{
		delete m_first_pass;
		m_first_pass = nullptr;
	}


	ColorSceneRenderer::ColorSceneRenderer()
	{
		m_clear_pass        = create_pass<ClearPass>();
		m_geometry_pass     = create_pass<GeometryPass>();
		m_deferred_pass     = create_pass<DeferredPass>();
		m_post_process_pass = create_pass<PostProcessPass>();
		m_overlay_pass      = create_pass<OverlayPass>();
	}
}// namespace Engine
