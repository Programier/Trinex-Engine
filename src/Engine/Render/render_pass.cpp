#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
	implement_struct_default_init(Engine::RenderPass, 0);

	RenderPass::RenderPass()
	{}

	RenderPass::~RenderPass()
	{
		if (m_next)
			delete m_next;
	}

	Refl::Struct* RenderPass::struct_instance() const
	{
		return static_struct_instance();
	}

	Refl::RenderPassInfo* RenderPass::info() const
	{
		return Refl::Object::instance_cast<Refl::RenderPassInfo>(struct_instance());
	}

	bool RenderPass::is_empty() const
	{
		return m_allocated == 0;
	}

	RenderPass& RenderPass::clear()
	{
		m_allocated = 0;
		return *this;
	}

	template<typename NodeType>
	static void render_octree_bounding_box(NodeType* node, BatchedLines& lines)
	{
		if (node == nullptr)
			return;

		node->box().write_to_batcher(lines);

		for (byte i = 0; i < 8; i++)
		{
			render_octree_bounding_box(node->child_at(i), lines);
		}
	}


	RenderPass& RenderPass::render(RenderViewport* render_target)
	{
		size_t offset = 0;
		byte* data    = m_commands.data();

		while (offset < m_allocated)
		{
			TaskInterface* task = reinterpret_cast<TaskInterface*>(data + offset);
			offset += task->size();
			task->execute();
		}

		return *this;
	}

	SceneRenderer* RenderPass::scene_renderer() const
	{
		return m_renderer;
	}

	RenderPass* RenderPass::next() const
	{
		return m_next;
	}

	// RENDER COMMANDS IMPLEMENTATION

#define declare_command_one_param(command_name, type1, name1, code)                                                              \
	class command_name##Command : public Task<command_name##Command>                                                             \
	{                                                                                                                            \
		type1 name1;                                                                                                             \
																																 \
	public:                                                                                                                      \
		command_name##Command(type1 var1) : name1(var1)                                                                          \
		{}                                                                                                                       \
																																 \
		void execute() override                                                                                                  \
		{                                                                                                                        \
			code;                                                                                                                \
		}                                                                                                                        \
	};

#define declare_command_two_param(command_name, type1, name1, type2, name2, code)                                                \
	class command_name##Command : public Task<command_name##Command>                                                             \
	{                                                                                                                            \
		type1 name1;                                                                                                             \
		type2 name2;                                                                                                             \
																																 \
	public:                                                                                                                      \
		command_name##Command(type1 var1, type2 var2) : name1(var1), name2(var2)                                                 \
		{}                                                                                                                       \
																																 \
		void execute() override                                                                                                  \
		{                                                                                                                        \
			code;                                                                                                                \
		}                                                                                                                        \
	};

#define declare_command_three_param(command_name, type1, name1, type2, name2, type3, name3, code)                                \
	class command_name##Command : public Task<command_name##Command>                                                             \
	{                                                                                                                            \
		type1 name1;                                                                                                             \
		type2 name2;                                                                                                             \
		type3 name3;                                                                                                             \
																																 \
	public:                                                                                                                      \
		command_name##Command(type1 var1, type2 var2, type3 var3) : name1(var1), name2(var2), name3(var3)                        \
		{}                                                                                                                       \
																																 \
		void execute() override                                                                                                  \
		{                                                                                                                        \
			code;                                                                                                                \
		}                                                                                                                        \
	};

#define declare_command_four_param(command_name, type1, name1, type2, name2, type3, name3, type4, name4, code)                   \
	class command_name##Command : public Task<command_name##Command>                                                             \
	{                                                                                                                            \
		type1 name1;                                                                                                             \
		type2 name2;                                                                                                             \
		type3 name3;                                                                                                             \
		type4 name4;                                                                                                             \
																																 \
	public:                                                                                                                      \
		command_name##Command(type1 var1, type2 var2, type3 var3, type4 var4)                                                    \
			: name1(var1), name2(var2), name3(var3), name4(var4)                                                                 \
		{}                                                                                                                       \
																																 \
		void execute() override                                                                                                  \
		{                                                                                                                        \
			code;                                                                                                                \
		}                                                                                                                        \
	};

	declare_command_two_param(Draw, size_t, vertices_count, size_t, vertices_offset, rhi->draw(vertices_count, vertices_offset));
	declare_command_three_param(DrawIndexed, size_t, indices_count, size_t, indices_offset, size_t, vertices_offset,
								rhi->draw_indexed(indices_count, indices_offset, vertices_offset));
	declare_command_three_param(DrawInstanced, size_t, vertices_count, size_t, vertices_offset, size_t, instances,
								rhi->draw_instanced(vertices_count, vertices_offset, instances));
	declare_command_four_param(DrawIndexedInstanced, size_t, indices_count, size_t, indices_offset, size_t, vertices_offset,
							   size_t, instances,
							   rhi->draw_indexed_instanced(indices_count, indices_offset, vertices_offset, instances));
	declare_command_three_param(BindMaterial, RenderPass*, render_pass, MaterialInterface*, interface, SceneComponent*, component,
								interface->apply(component, render_pass));

	declare_command_three_param(BindVertexBuffer, VertexBuffer*, buffer, byte, stream, size_t, offset,
								buffer->rhi_bind(stream, offset));

	declare_command_two_param(BindIndexBuffer, IndexBuffer*, buffer, size_t, offset, buffer->rhi_bind(offset));

	RenderPass& RenderPass::draw(size_t vertices_count, size_t vertices_offset)
	{
		create_command<DrawCommand>(vertices_count, vertices_offset);
		return *this;
	}

	RenderPass& RenderPass::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
	{
		create_command<DrawIndexedCommand>(indices_count, indices_offset, vertices_offset);
		return *this;
	}

	RenderPass& RenderPass::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		create_command<DrawInstancedCommand>(vertex_count, vertices_offset, instances);
		return *this;
	}

	RenderPass& RenderPass::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
												   size_t instances)
	{
		create_command<DrawIndexedInstancedCommand>(indices_count, indices_offset, vertices_offset, instances);
		return *this;
	}

	RenderPass& RenderPass::bind_material(class MaterialInterface* material, SceneComponent* component)
	{
		create_command<BindMaterialCommand>(this, material, component);
		return *this;
	}

	RenderPass& RenderPass::bind_vertex_buffer(class VertexBuffer* buffer, byte stream, size_t offset)
	{
		create_command<BindVertexBufferCommand>(buffer, stream, offset);
		return *this;
	}

	RenderPass& RenderPass::bind_index_buffer(class IndexBuffer* buffer, size_t offset)
	{
		create_command<BindIndexBufferCommand>(buffer, offset);
		return *this;
	}

	// IMPLEMENTATION OF RENDER PASSES

	trinex_impl_render_pass(Engine::ClearPass)
	{}

	trinex_impl_render_pass(Engine::DepthPass)
	{
		info.entry     = "depth";
		info.has_depth = true;
	}

	trinex_impl_render_pass(Engine::ShadowPass)
	{}

	trinex_impl_render_pass(Engine::GeometryPass)
	{}

	trinex_impl_render_pass(Engine::ForwardPass)
	{}

	trinex_impl_render_pass(Engine::DeferredLightingPass)
	{
		info.shader_definitions = {
				{"TRINEX_DEFERRED_LIGHTING_PASS", "1"},
		};

		info.entry                   = "deferred_light";
		info.color_attachments_count = 1;
	}

	trinex_impl_render_pass(Engine::TransparencyPass)
	{}

	trinex_impl_render_pass(Engine::PostProcessPass)
	{}

	trinex_impl_render_pass(Engine::OverlayPass)
	{}

	bool ClearPass::is_empty() const
	{
		return false;
	}

	ClearPass& ClearPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->clear();
		Super::render(vp);
		return *this;
	}

	GeometryPass& GeometryPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_gbuffer();
		Super::render(vp);
		return *this;
	}

	bool DeferredLightingPass::is_empty() const
	{
		return is_not_in<ViewMode::Lit>(scene_renderer()->view_mode());
	}

	DeferredLightingPass& DeferredLightingPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_scene_color_ldr(false);

		auto renderer = scene_renderer();

		if (renderer->view_mode() == ViewMode::Unlit)
		{
		}
		else if (renderer->view_mode() == ViewMode::Lit)
		{
			static Name name_ambient_color = "ambient_color";
			Material* material             = DefaultResources::Materials::ambient_light;

			if (material)
			{
				auto ambient_param =
						Object::instance_cast<MaterialParameters::Float3>(material->find_parameter(name_ambient_color));

				if (ambient_param)
				{
					ambient_param->value = renderer->scene->environment.ambient_color;
				}

				material->apply();
				rhi->draw(6, 0);
			}
		}

		Super::render(vp);
		return *this;
	}

	bool PostProcessPass::is_empty() const
	{
		return is_not_in<ViewMode::Lit>(scene_renderer()->view_mode());
	}

	PostProcessPass& PostProcessPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_scene_color_ldr(false);
		Super::render(vp);
		return *this;
	}

	bool OverlayPass::is_empty() const
	{
		return false;
	}

	OverlayPass& OverlayPass::clear()
	{
		Super::clear();
		lines.clear();
		triangles.clear();
		return *this;
	}

	OverlayPass& OverlayPass::copy_view_texture(ViewMode mode)
	{
		RenderSurface* surface = nullptr;
		auto render_targets    = SceneRenderTargets::instance();

		switch (mode)
		{
			case ViewMode::Unlit:
				surface = render_targets->surface_of(SceneRenderTargets::BaseColor);
				break;

			case ViewMode::WorldNormal:
				surface = render_targets->surface_of(SceneRenderTargets::Normal);
				break;

			default:
				return *this;
		}

		auto renderer = scene_renderer();
		auto& params  = renderer->global_shader_parameters();
		auto& vp      = params.viewport;

		auto min = Vector2D(vp.x, vp.y) / params.size;
		auto max = Vector2D(vp.x + vp.z, vp.y + vp.w) / params.size;
		renderer->blit(reinterpret_cast<Texture2D*>(surface), min, max);

		return *this;
	}

	OverlayPass& OverlayPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_scene_color_ldr(false);

		{
			auto mode = scene_renderer()->view_mode();

			if (mode != ViewMode::Lit)
			{
				copy_view_texture(mode);
			}
		}

		Super::render(vp);

		auto renderer = scene_renderer();

		if ((renderer->scene_view().show_flags() & ShowFlags::LightOctree) != ShowFlags::None)
		{
			render_octree_bounding_box(renderer->scene->primitive_octree().root_node(), lines);
		}

		if ((renderer->scene_view().show_flags() & ShowFlags::PrimitiveOctree) != ShowFlags::None)
		{
			render_octree_bounding_box(renderer->scene->light_octree().root_node(), lines);
		}

		lines.render(renderer->scene_view());
		triangles.render(renderer->scene_view());
		return *this;
	}
}// namespace Engine
