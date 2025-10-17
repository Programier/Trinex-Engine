#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/thread.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <Graphics/editor_pipelines.hpp>
#include <Graphics/editor_render_passes.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_bindings.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	class HitproxyRenderer : public Renderer
	{
	public:
		HitproxyRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit) : Renderer(scene, view, mode) {}

		RHITexture* render_hitproxies()
		{
			Vector2u size                               = scene_view().view_size();
			FrameVector<PrimitiveComponent*> primitives = scene()->collect_visible_primitives(scene_view().projview());

			auto pool    = RHITexturePool::global_instance();
			auto surface = pool->request_transient_surface(RHISurfaceFormat::RG32UI, size);
			auto depth   = scene_depth_target();

			auto surface_rtv = surface->as_rtv();
			auto depth_dsv   = depth->as_dsv();

			trinex_rhi_push_stage("HitProxy");

			rhi->context()->clear_irtv(surface_rtv).clear_dsv(depth_dsv);
			rhi->context()->bind_render_target1(surface_rtv, depth_dsv);
			rhi->context()->viewport(RHIViewport(size));
			rhi->context()->scissor(RHIScissors(size));

			static MaterialBindings bindings;
			static MaterialBindings::Binding* proxy_id = bindings.find_or_create("hitproxy.id");

			for (PrimitiveComponent* primitive : primitives)
			{
				if (Actor* actor = primitive->actor())
				{
					uintptr_t addr = reinterpret_cast<uintptr_t>(actor);

					Vector2u id;
					id.x        = static_cast<uint32_t>(addr & 0xFFFFFFFFu);
					id.y        = static_cast<uint32_t>((addr >> 32) & 0xFFFFFFFFu);
					(*proxy_id) = id;

					primitive->proxy()->render(this, EditorRenderPasses::HitProxy::static_instance(), &bindings);
				}
			}

			trinex_rhi_pop_stage();
			return surface;
		}
	};


	EditorRenderer::EditorRenderer(Scene* scene, const SceneView& view, ViewMode mode) : DeferredRenderer(scene, view, mode) {}

	Actor* EditorRenderer::static_raycast(const SceneView& view, Vector2f uv, Scene* scene)
	{
		uv = glm::clamp(uv, Vector2f(0.f), Vector2f(1.f));
		HitproxyRenderer renderer(scene, view);
		RHITexture* hitproxy = renderer.render_hitproxies();

		auto buffer_pool = RHIBufferPool::global_instance();
		auto fence_pool  = RHIFencePool::global_instance();

		static constexpr RHIBufferCreateFlags buffer_flags = RHIBufferCreateFlags::TransferDst | RHIBufferCreateFlags::CPURead;

		auto buffer = buffer_pool->request_buffer(8, buffer_flags);
		auto fence  = fence_pool->request_fence();

		Vector2f size = view.view_size();
		Vector3u offset(static_cast<uint32_t>((size.x * uv.x) + 0.5f), static_cast<uint32_t>((size.y * uv.y) + 0.5f), 0);
		rhi->context()->copy_texture_to_buffer(hitproxy, 0, 0, offset, {1, 1, 1}, buffer, 0);

		rhi->signal(fence);

		while (!fence->is_signaled()) Thread::static_yield();

		Actor* actor = nullptr;
		byte* data   = buffer->map();
		std::memcpy(&actor, data, 8);
		buffer->unmap();

		RHIFencePool::global_instance()->return_fence(fence);
		RHIBufferPool::global_instance()->return_buffer(buffer);
		return actor;
	}

	EditorRenderer& EditorRenderer::render_grid()
	{
		if (!Settings::Editor::show_grid)
			return *this;

		render_graph()
		        ->add_pass(RenderGraph::Pass::Graphics, "Editor Grid")
		        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this]() {
			        rhi->context()->bind_render_target1(scene_color_ldr_target()->as_rtv(), scene_depth_target()->as_dsv());
			        EditorPipelines::Grid::instance()->render(this);
		        });
		return *this;
	}

	void EditorRenderer::render_outlines_pass(Actor** actors, size_t count)
	{
		auto view_size = scene_view().view_size();
		auto pool      = RHITexturePool::global_instance();
		auto depth     = pool->request_surface(RHISurfaceFormat::D32F, view_size);

		auto dsv = depth->as_dsv();
		rhi->context()->clear_dsv(dsv).bind_depth_stencil_target(dsv);

		for (size_t i = 0; i < count; ++i)
		{
			Actor* actor = actors[i];

			for (ActorComponent* component : actor->owned_components())
			{
				if (auto primitive = Object::instance_cast<PrimitiveComponent>(component))
				{
					primitive->proxy()->render(this, RenderPasses::Depth::static_instance());
				}
			}
		}

		EditorPipelines::Outline::instance()->render(this, depth->as_srv(), {1.f, 0.f, 0.f});
		pool->return_surface(depth);
	}

	EditorRenderer& EditorRenderer::render_outlines(Actor** actors, size_t count)
	{
		if (actors && count)
		{
			render_graph()
			        ->add_pass(RenderGraph::Pass::Graphics, "Editor Outlines")
			        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
			        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
			        .add_func([this, actors, count]() { render_outlines_pass(actors, count); });
		}

		return *this;
	}

	EditorRenderer& EditorRenderer::render_primitives(Actor** actors, size_t count)
	{
		FrameVector<LightComponent*> lights = visible_lights();

		for (LightComponent* light : lights)
		{
			if (!light->actor()->is_selected())
				continue;

			if (auto spot_light = Object::instance_cast<SpotLightComponent>(light))
			{
				auto proxy = spot_light->proxy();

				auto dir           = proxy->direction() * 2.f;
				float outer_radius = 2.f * glm::tan(proxy->outer_cone_angle());
				float inner_radius = 2.f * glm::tan(proxy->inner_cone_angle());

				auto location = proxy->world_transform().location() + dir;
				lines.add_cone(location, -dir, outer_radius, {255, 255, 0, 255}, 0, 3.f);
				lines.add_cone(location, -dir, inner_radius, {255, 255, 0, 255}, 0, 3.f);
			}
			else if (auto point_light = Object::instance_cast<PointLightComponent>(light))
			{
				auto proxy     = point_light->proxy();
				auto& location = proxy->world_transform().location();
				lines.add_sphere(location, proxy->attenuation_radius(), {255, 255, 0, 255}, 0, 3.f);
			}
			else if (auto directional_light = Object::instance_cast<DirectionalLightComponent>(light))
			{
				auto proxy     = directional_light->proxy();
				auto& location = proxy->world_transform().location();
				lines.add_arrow(location, proxy->direction(), {255, 255, 0, 255}, 3.f);
			}
		}

		return *this;
	}
}// namespace Engine
