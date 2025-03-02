#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
	struct GlobalUniformBuffer final {
		GlobalShaderParameters storage;
		GlobalUniformBuffer* prev         = nullptr;
		RHI_UniformBuffer* uniform_buffer = nullptr;

		GlobalUniformBuffer()
		{
			uniform_buffer = rhi->create_uniform_buffer(sizeof(GlobalShaderParameters), nullptr, RHIBufferType::Dynamic);
		}

		delete_copy_constructors(GlobalUniformBuffer);

		inline void rhi_update()
		{
			uniform_buffer->update(0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&storage));
		}

		~GlobalUniformBuffer()
		{
			uniform_buffer->release();
		}
	};

	class GlobalShaderParametersManager
	{
		Vector<GlobalUniformBuffer*> m_uniform_buffers;
		Vector<GlobalUniformBuffer*> m_used_buffers;
		GlobalUniformBuffer* m_current = nullptr;

		GlobalUniformBuffer* setup_buffer(GlobalUniformBuffer* buffer)
		{
			buffer->prev = m_current;
			m_current    = buffer;
			return buffer;
		}

	public:
		GlobalUniformBuffer* push()
		{
			if (!m_uniform_buffers.empty())
			{
				GlobalUniformBuffer* buffer = m_uniform_buffers.back();
				m_uniform_buffers.pop_back();
				m_used_buffers.push_back(buffer);
				return setup_buffer(buffer);
			}

			GlobalUniformBuffer* buffer = new GlobalUniformBuffer();
			m_used_buffers.push_back(buffer);
			return setup_buffer(buffer);
		}

		GlobalUniformBuffer* push(const GlobalShaderParameters& params)
		{
			auto buffer     = push();
			buffer->storage = params;
			buffer->rhi_update();
			return buffer;
		}

		GlobalUniformBuffer* pop()
		{
			if (m_current)
			{
				GlobalUniformBuffer* tmp = m_current;
				m_current                = tmp->prev;
				tmp->prev                = nullptr;
			}

			return m_current;
		}

		GlobalUniformBuffer* current()
		{
			return m_current;
		}

		GlobalShaderParametersManager& reset()
		{
			for (GlobalUniformBuffer* buffer : m_used_buffers)
			{
				m_uniform_buffers.push_back(buffer);
			}
			m_uniform_buffers.clear();
			return *this;
		}

		~GlobalShaderParametersManager()
		{
			for (auto buffer : m_uniform_buffers)
			{
				delete buffer;
			}

			for (auto buffer : m_used_buffers)
			{
				delete buffer;
			}
		}
	};

	SceneRenderer::SceneRenderer() : m_global_shader_params(new GlobalShaderParametersManager()), scene(nullptr)
	{}

	SceneRenderer& SceneRenderer::initialize()
	{
		return *this;
	}

	SceneRenderer& SceneRenderer::finalize()
	{
		return *this;
	}

	const GlobalShaderParameters& SceneRenderer::global_parameters() const
	{
		GlobalUniformBuffer* buffer = m_global_shader_params->current();
		if (!buffer)
			throw EngineException("Shader parameters stack is empty!");
		return buffer->storage;
	}

	const SceneRenderer& SceneRenderer::bind_global_parameters(BindingIndex index) const
	{
		GlobalUniformBuffer* buffer = m_global_shader_params->current();
		if (!buffer)
			throw EngineException("Shader parameters stack is empty!");
		buffer->uniform_buffer->bind(index);
		return *this;
	}

	const SceneView& SceneRenderer::scene_view() const
	{
		if (m_scene_views.empty())
			throw EngineException("Scene Views stack is empty!");
		return m_scene_views.back();
	}

	SceneRenderer& SceneRenderer::push_global_parameters(GlobalShaderParameters* parameters)
	{
		if (parameters)
		{
			m_global_shader_params->push(*parameters);
			return *this;
		}

		const SceneView& view = scene_view();
		auto buffer           = m_global_shader_params->push();
		buffer->storage.update(&view, SceneRenderTargets::instance()->size());
		buffer->rhi_update();
		return *this;
	}

	SceneRenderer& SceneRenderer::pop_global_parameters()
	{
		m_global_shader_params->pop();
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

	SceneRenderer* SceneRenderer::static_scene_renderer_of(RenderPass* pass)
	{
		return pass ? pass->scene_renderer() : nullptr;
	}

	void SceneRenderer::create_pass_internal(RenderPass* next, RenderPass* current)
	{
		current->m_renderer = this;
		current->m_next     = next;

		if (next == m_first_pass)
		{
			m_first_pass = current;
		}

		if (next == nullptr)
		{
			if (m_last_pass)
			{
				m_last_pass->m_next = current;
			}
			m_last_pass = current;
		}

		current->initialize();
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

	SceneRenderer& SceneRenderer::blit(class Texture2D* texture, const Vector2f& min, const Vector2f& max)
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

		m_global_shader_params->reset();
		m_scene_views.clear();
		m_scene_views.push_back(view);

		rhi->viewport(view.viewport());
		rhi->scissor(view.scissor());

		push_global_parameters();

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

#if TRINEX_DEBUG_BUILD
			rhi->pop_debug_stage();
#endif
		}

		pop_global_parameters();
		m_scene_views.pop_back();

		return *this;
	}

	SceneRenderer& SceneRenderer::render_component(PrimitiveComponent* component)
	{
		return *this;
	}

	SceneRenderer::~SceneRenderer()
	{
		delete m_first_pass;
		m_first_pass = nullptr;
		delete m_global_shader_params;
	}

	DepthSceneRenderer& DepthSceneRenderer::initialize()
	{
		create_pass<DepthPass>();
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::initialize()
	{
		m_shadow_pass            = create_pass<ShadowPass>();
		m_clear_pass             = create_pass<ClearPass>();
		m_geometry_pass          = create_pass<GeometryPass>();
		m_deferred_lighting_pass = create_pass<DeferredLightingPass>();
		m_post_process_pass      = create_pass<PostProcessPass>();
		m_overlay_pass           = create_pass<OverlayPass>();

		initialize_subrenderers();
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::finalize()
	{
		finalize_subrenderers();
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::initialize_subrenderers()
	{
		m_depth_renderer = new Renderer<DepthSceneRenderer>();
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::finalize_subrenderers()
	{
		delete m_depth_renderer;
		m_depth_renderer = nullptr;
		return *this;
	}
}// namespace Engine
