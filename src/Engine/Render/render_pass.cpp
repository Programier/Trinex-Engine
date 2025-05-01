#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
	trinex_implement_struct_default_init(Engine::RenderPass, 0);

	Refl::RenderPassInfo* RenderPass::static_cast_to_render_pass_info(Refl::Struct* info)
	{
		return Refl::Object::instance_cast<Refl::RenderPassInfo>(info);
	}

	RenderPass::RenderPass() {}

	RenderPass::~RenderPass()
	{
		if (m_childs)
			delete m_childs;

		if (m_next)
			delete m_next;
	}

	RenderPass& RenderPass::initialize()
	{
		return *this;
	}

	Refl::Struct* RenderPass::struct_instance() const
	{
		return static_struct_instance();
	}

	Refl::RenderPassInfo* RenderPass::info() const
	{
		return Refl::Object::instance_cast<Refl::RenderPassInfo>(struct_instance());
	}

	void RenderPass::initialize_subpass(RenderPass* pass)
	{
		pass->m_owner    = this;
		pass->m_renderer = m_renderer;

		RenderPass** node = &m_childs;
		while (*node) node = &(*node)->m_next;
		*node = pass;

		pass->initialize();
	}

	bool RenderPass::destroy_subpass(RenderPass* pass)
	{
		if (pass->m_owner != this)
			return false;

		RenderPass** node = &m_childs;
		while (*node && *node != pass) node = &(*node)->m_next;

		if (*node)
		{
			*node = pass->m_next;
			delete pass;
			return true;
		}

		return false;
	}

	bool RenderPass::is_empty() const
	{
		return m_allocated == 0;
	}

	RenderPass& RenderPass::clear()
	{
		for (auto child = m_childs; child; child = child->m_next)
		{
			child->clear();
		}

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

	RenderPass& RenderPass::render(RenderViewport* vp)
	{
		size_t offset = 0;
		byte* data    = m_commands.data();

		// Render child passes

		for (auto child = m_childs; child; child = child->m_next)
		{
#if TRINEX_DEBUG_BUILD
			rhi->push_debug_stage(child->struct_instance()->name().c_str());
#endif
			child->render(vp);

#if TRINEX_DEBUG_BUILD
			rhi->pop_debug_stage();
#endif
		}

		while (offset < m_allocated)
		{
			TaskInterface* task = reinterpret_cast<TaskInterface*>(data + offset);
			offset += align_up(task->size(), command_alignment);
			task->execute();
		}

		return *this;
	}

	RenderPass& RenderPass::predraw(PrimitiveComponent* primitive, MaterialInterface* material, Pipeline* pipeline)
	{
		return *this;
	}

	RenderPass& RenderPass::draw(size_t vertices_count, size_t vertices_offset)
	{
		add_callabble([=]() { rhi->draw(vertices_count, vertices_offset); });
		return *this;
	}

	RenderPass& RenderPass::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
	{
		add_callabble([=]() { rhi->draw_indexed(indices_count, indices_offset, vertices_offset); });
		return *this;
	}

	RenderPass& RenderPass::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		add_callabble([=]() { rhi->draw_instanced(vertex_count, vertices_offset, instances); });
		return *this;
	}

	RenderPass& RenderPass::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
	                                               size_t instances)
	{
		add_callabble([=]() { rhi->draw_indexed_instanced(indices_count, indices_offset, vertices_offset, instances); });
		return *this;
	}

	RenderPass& RenderPass::bind_material(class MaterialInterface* material, SceneComponent* component)
	{
		add_callabble([=, self = this]() { material->apply(component, self); });
		return *this;
	}

	RenderPass& RenderPass::bind_vertex_buffer(class VertexBufferBase* buffer, byte stream, size_t offset)
	{
		add_callabble([=]() { buffer->rhi_bind(stream, offset); });
		return *this;
	}

	RenderPass& RenderPass::bind_index_buffer(class IndexBuffer* buffer, size_t offset)
	{
		add_callabble([=]() { buffer->rhi_bind(offset); });
		return *this;
	}

	// IMPLEMENTATION OF RENDER PASSES

	static bool is_opaque_material(const Material* material)
	{
		if (material->domain != MaterialDomain::Surface)
			return false;

		auto desc = material->graphics_description();

		if (desc == nullptr)
			return false;

		if (desc->color_blending.enable)
		{
			return false;
		}

		if (!desc->depth_test.enable || !desc->depth_test.write_enable)
		{
			return false;
		}

		if (desc->rasterizer.polygon_mode != PolygonMode::Fill)
		{
			return false;
		}

		return true;
	}

	trinex_impl_render_pass(Engine::ClearPass) {}

	trinex_impl_render_pass(Engine::DepthPass)
	{
		m_attachments_count = 0;

		m_shader_definitions = {
		        {"TRINEX_DEPTH_PASS", "1"},
		};

		m_is_material_compatible = is_opaque_material;
	}

	trinex_impl_render_pass(Engine::GeometryPass)
	{
		m_attachments_count = 4;

		m_shader_definitions = {
		        {"TRINEX_GEOMETRY_PASS", "1"},
		};

		m_is_material_compatible = is_opaque_material;
	}

	trinex_impl_render_pass(Engine::TransparencyPass) {}

	trinex_impl_render_pass(Engine::PostProcessPass) {}

	trinex_impl_render_pass(Engine::OverlayPass) {}

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

	bool PostProcessPass::is_empty() const
	{
		return is_not_in<ViewMode::Lit>(scene_renderer()->view_mode());
	}

	PostProcessPass& PostProcessPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_scene_color(false);
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
		RenderSurface* src  = nullptr;
		auto render_targets = SceneRenderTargets::instance();
		Swizzle swizzle;

		switch (mode)
		{
			case ViewMode::Unlit:
				src     = render_targets->surface_of(SceneRenderTargets::BaseColor);
				swizzle = {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::A};
				break;

			case ViewMode::WorldNormal:
				src     = render_targets->surface_of(SceneRenderTargets::Normal);
				swizzle = {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One};
				break;

			case ViewMode::Metalic:
				src     = render_targets->surface_of(SceneRenderTargets::MSRA);
				swizzle = {Swizzle::R, Swizzle::R, Swizzle::R, Swizzle::One};
				break;

			case ViewMode::Specular:
				src     = render_targets->surface_of(SceneRenderTargets::MSRA);
				swizzle = {Swizzle::G, Swizzle::G, Swizzle::G, Swizzle::One};
				break;

			case ViewMode::Roughness:
				src     = render_targets->surface_of(SceneRenderTargets::MSRA);
				swizzle = {Swizzle::B, Swizzle::B, Swizzle::B, Swizzle::One};
				break;

			default: return *this;
		}

		Rect2D rect;
		rect.pos  = {0, 0};
		rect.size = src->size();

		auto dst = render_targets->surface_of(SceneRenderTargets::SceneColor)->rhi_uav();
		Pipelines::Blit2D::instance()->blit(src->rhi_srv(), dst, rect, rect, 0, swizzle);
		return *this;
	}

	OverlayPass& OverlayPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_scene_color();

		{
			auto mode = scene_renderer()->view_mode();

			if (mode != ViewMode::Lit)
			{
				copy_view_texture(mode);
			}
		}

		Super::render(vp);

		auto renderer = scene_renderer();

		if ((renderer->scene_view().show_flags() & ShowFlags::PrimitiveOctree) != ShowFlags::None)
		{
			render_octree_bounding_box(renderer->scene->primitive_octree().root_node(), lines);
		}

		if ((renderer->scene_view().show_flags() & ShowFlags::LightOctree) != ShowFlags::None)
		{
			render_octree_bounding_box(renderer->scene->light_octree().root_node(), lines);
		}

		SceneRenderTargets::instance()->bind_scene_color(true);
		lines.render(this);
		triangles.render(this);
		return *this;
	}
}// namespace Engine
