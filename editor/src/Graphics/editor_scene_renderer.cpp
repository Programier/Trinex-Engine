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
#include <RHI/rhi.hpp>

namespace Engine::EditorRenderer
{
	class EditorRenderer : public Renderer
	{
	public:
		EditorRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit) : Renderer(scene, view, mode) {}

		RHITexture* render_hitproxies()
		{
			Vector2u size                               = scene_view().view_size();
			FrameVector<PrimitiveComponent*> primitives = scene()->collect_visible_primitives(scene_view().camera_view());

			auto surface = RHISurfacePool::global_instance()->request_transient_surface(RHISurfaceFormat::RG32UI, size);
			auto depth   = scene_depth_target();

			auto surface_rtv = surface->as_rtv();
			auto depth_dsv   = depth->as_dsv();

			trinex_rhi_push_stage("HitProxy");
			surface_rtv->clear_uint({0, 0, 0, 0});
			depth_dsv->clear(1.f, 0);

			rhi->bind_render_target1(surface_rtv, depth_dsv);
			rhi->viewport(RHIViewport(size));
			rhi->scissor(RHIScissors(size));

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

					render_primitive(EditorRenderPasses::HitProxy::static_instance(), primitive, &bindings);
				}
			}

			trinex_rhi_pop_stage();
			return surface;
		}
	};

	void render_grid(Renderer* renderer)
	{
		if (!Settings::Editor::show_grid)
			return;

		renderer->render_graph()
		        ->add_pass(RenderGraph::Pass::Graphics, "Editor Grid")
		        .add_resource(renderer->scene_color_ldr_target(), RHIAccess::RTV)
		        .add_resource(renderer->scene_depth_target(), RHIAccess::DSV)
		        .add_func([renderer]() {
			        rhi->bind_render_target1(renderer->scene_color_ldr_target()->as_rtv(),
			                                 renderer->scene_depth_target()->as_dsv());
			        EditorPipelines::Grid::instance()->render(renderer);
		        });
	}

	static void render_outlines_pass(Renderer* renderer, Actor** actors, size_t count)
	{
		auto view_size = renderer->scene_view().view_size();
		auto pool      = RHISurfacePool::global_instance();
		auto depth     = pool->request_surface(RHISurfaceFormat::D32F, view_size);

		depth->as_dsv()->clear(1.f, 0);
		rhi->bind_depth_stencil_target(depth->as_dsv());

		for (size_t i = 0; i < count; ++i)
		{
			Actor* actor = actors[i];

			for (ActorComponent* component : actor->owned_components())
			{
				if (auto primitive = Object::instance_cast<PrimitiveComponent>(component))
				{
					renderer->render_primitive(RenderPasses::Depth::static_instance(), primitive);
				}
			}
		}

		EditorPipelines::Outline::instance()->render(renderer, depth->as_srv(), {1.f, 0.f, 0.f});
		pool->return_surface(depth);
	}

	void render_outlines(Renderer* renderer, Actor** actors, size_t count)
	{
		renderer->render_graph()
		        ->add_pass(RenderGraph::Pass::Graphics, "Editor Outlines")
		        .add_resource(renderer->scene_color_ldr_target(), RHIAccess::RTV)
		        .add_resource(renderer->scene_depth_target(), RHIAccess::SRVGraphics)
		        .add_func([renderer, actors, count]() { render_outlines_pass(renderer, actors, count); });
	}

	void render_primitives(Renderer* renderer, Actor** actors, size_t count)
	{
		Frustum frustum                     = renderer->scene_view().camera_view();
		FrameVector<LightComponent*> lights = renderer->scene()->collect_visible_lights(frustum);

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
				renderer->lines.add_cone(location, -dir, outer_radius, {255, 255, 0, 255}, 0, 3.f);
				renderer->lines.add_cone(location, -dir, inner_radius, {255, 255, 0, 255}, 0, 3.f);
			}
			else if (auto point_light = Object::instance_cast<PointLightComponent>(light))
			{
				auto proxy     = point_light->proxy();
				auto& location = proxy->world_transform().location();
				renderer->lines.add_sphere(location, proxy->attenuation_radius(), {255, 255, 0, 255}, 0, 3.f);
			}
			else if (auto directional_light = Object::instance_cast<DirectionalLightComponent>(light))
			{
				auto proxy     = directional_light->proxy();
				auto& location = proxy->world_transform().location();
				renderer->lines.add_arrow(location, proxy->direction(), {255, 255, 0, 255}, 3.f);
			}
		}
	}

	Actor* raycast(const SceneView& view, Vector2f uv, Scene* scene)
	{
		uv = glm::clamp(uv, Vector2f(0.f), Vector2f(1.f));
		EditorRenderer renderer(scene, view);
		RHITexture* hitproxy = renderer.render_hitproxies();

		auto buffer_pool = RHIBufferPool::global_instance();
		auto fence_pool  = RHIFencePool::global_instance();

		static constexpr RHIBufferCreateFlags buffer_flags = RHIBufferCreateFlags::TransferDst | RHIBufferCreateFlags::CPURead;

		auto buffer = buffer_pool->request_buffer(8, buffer_flags);
		auto fence  = fence_pool->request_fence();

		Vector2f size = view.view_size();
		Vector3u offset(static_cast<uint32_t>((size.x * uv.x) + 0.5f), static_cast<uint32_t>((size.y * uv.y) + 0.5f), 0);
		rhi->copy_texture_to_buffer(hitproxy, 0, 0, offset, {1, 1, 1}, buffer, 0);

		rhi->signal_fence(fence);
		rhi->submit();

		while (!fence->is_signaled()) Thread::static_yield();

		Actor* actor = nullptr;
		byte* data   = buffer->map();
		std::memcpy(&actor, data, 8);
		buffer->unmap();

		RHIFencePool::global_instance()->return_fence(fence);
		RHIBufferPool::global_instance()->return_buffer(buffer);
		return actor;
	}
}// namespace Engine::EditorRenderer
