#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/threading.hpp>
#include <Editor/engine.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/primitive_context.hpp>
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

		RHITexture* render_hitproxies(RHIContext* ctx)
		{
			Vector2u size                               = scene_view().view_size();
			const Matrix4f& projview                    = scene_view().camera_view().projview;
			FrameVector<PrimitiveComponent*> primitives = scene()->collect_visible_primitives(projview);

			auto pool    = RHITexturePool::global_instance();
			auto surface = pool->request_transient_surface(RHISurfaceFormat::RG32UI, size);
			auto depth   = scene_depth_target();

			auto surface_rtv = surface->as_rtv();
			auto depth_dsv   = depth->as_dsv();

			trinex_rhi_push_stage(ctx, "HitProxy");

			ctx->barrier(surface, RHIAccess::TransferDst);
			ctx->barrier(depth, RHIAccess::TransferDst);
			ctx->clear_irtv(surface_rtv);
			ctx->clear_dsv(depth_dsv);
			ctx->barrier(surface, RHIAccess::RTV);
			ctx->barrier(depth, RHIAccess::DSV);
			ctx->bind_render_target1(surface_rtv, depth_dsv);
			ctx->viewport(scene_view().viewport());
			ctx->scissor(scene_view().scissor());

			ctx->blending_state(RHIBlendingState::opaque);

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

					Matrix4f matrix  = primitive->world_transform().matrix();
					RenderPass* pass = EditorRenderPasses::HitProxy::static_instance();
					PrimitiveRenderingContext context(this, ctx, pass, &matrix, &bindings);
					primitive->render(&context);
				}
			}

			trinex_rhi_pop_stage(ctx);
			return surface;
		}
	};

	EditorRenderer::EditorRenderer(Scene* scene, const SceneView& view, ViewMode mode) : DeferredRenderer(scene, view, mode) {}

	Actor* EditorRenderer::static_raycast(const SceneView& view, Vector2f uv, Scene* scene)
	{
		uv = glm::clamp(uv, Vector2f(0.f), Vector2f(1.f));
		HitproxyRenderer renderer(scene, view);

		auto buffer_pool  = RHIBufferPool::global_instance();
		auto fence_pool   = RHIFencePool::global_instance();
		auto context_pool = RHIContextPool::global_instance();

		static constexpr RHIBufferCreateFlags buffer_flags = RHIBufferCreateFlags::TransferDst | RHIBufferCreateFlags::CPURead;

		auto buffer = buffer_pool->request_buffer(8, buffer_flags);
		auto fence  = fence_pool->request_fence();
		auto ctx    = context_pool->begin_context();

		RHITexture* hitproxy = renderer.render_hitproxies(ctx);

		Vector2f size = view.view_size();
		Vector3u offset(static_cast<uint32_t>((size.x * uv.x) + 0.5f), static_cast<uint32_t>((size.y * uv.y) + 0.5f), 0);

		ctx->barrier(hitproxy, RHIAccess::TransferSrc);
		ctx->barrier(buffer, RHIAccess::TransferDst);
		ctx->copy_texture_to_buffer(hitproxy, 0, 0, offset, {1, 1, 1}, buffer, 0);

		context_pool->end_context(ctx);
		rhi->signal(fence);

		while (!fence->is_signaled()) Thread::static_yield();

		byte* data   = buffer->map(RHIMappingAccess::Read);
		Actor* actor = *reinterpret_cast<Actor**>(data);
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
		        ->add_pass("Editor Grid")
		        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
		        .add_untracked_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_dependency(scene_depth_clear_pass())
		        .add_func([this](RHIContext* ctx) {
			        ctx->bind_render_target1(scene_color_ldr_target()->as_rtv(), scene_depth_target()->as_dsv());
			        EditorPipelines::Grid::instance()->render(ctx, this);
		        });
		return *this;
	}

	EditorRenderer& EditorRenderer::render_outlines(Actor* const* actors, size_t count)
	{
		if (actors && count)
		{
			auto view_size = scene_view().view_size();
			auto pool      = RHITexturePool::global_instance();
			auto depth     = pool->request_surface(RHISurfaceFormat::D32F, view_size);
			auto color     = pool->request_surface(static_surface_format_of(BaseColor), view_size);

			auto dsv = depth->as_dsv();

			render_graph()
			        ->add_pass("Outlines Clear Depth")
			        .add_resource(depth, RHIAccess::TransferDst)
			        .add_func([dsv](RHIContext* ctx) { ctx->clear_dsv(dsv); });

			auto& outlines_depth = render_graph()->add_pass("Outlines Depth");
			auto& outlines_color = render_graph()->add_pass("Outlines Color");

			outlines_depth.add_resource(depth, RHIAccess::DSV).add_func([this, dsv, actors, count](RHIContext* ctx) mutable {
				ctx->bind_depth_stencil_target(dsv);

				while (count-- > 0)
				{
					Actor* actor = *(actors++);
					for (ActorComponent* component : actor->owned_components())
					{
						if (auto primitive = Object::instance_cast<PrimitiveComponent>(component))
						{
							Matrix4f matrix  = primitive->world_transform().matrix();
							RenderPass* pass = RenderPasses::Depth::static_instance();
							PrimitiveRenderingContext context(this, ctx, pass, &matrix);
							primitive->render(&context);
						}
					}
				}
			});

			outlines_color.add_resource(color, RHIAccess::TransferDst)
			        .add_resource(scene_color_ldr_target(), RHIAccess::TransferSrc)
			        .add_func([this, color](RHIContext* ctx) {
				        RHITextureRegion region = {scene_view().view_size()};
				        ctx->copy_texture_to_texture(scene_color_ldr_target(), region, color, region);
			        });

			render_graph()
			        ->add_pass("Outlines")
			        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
			        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
			        .add_resource(color, RHIAccess::SRVGraphics)
			        .add_resource(depth, RHIAccess::SRVGraphics)
			        .add_func([this, color, depth](RHIContext* ctx) {
				        EditorPipelines::Outline::instance()->render(ctx, this, color->as_srv(), depth->as_srv(),
				                                                     {1.f, 0.f, 0.f});
				        RHITexturePool::global_instance()->return_surface(color);
				        RHITexturePool::global_instance()->return_surface(depth);
			        });
		}

		return *this;
	}

	EditorRenderer& EditorRenderer::render_primitives(Actor* const* actors, size_t count)
	{
		FrameVector<LightComponent*> lights = visible_lights();

		auto editor = EditorEngine::instance();

		for (LightComponent* light : lights)
		{
			if (!editor->is_selected(light->actor()))
				continue;

			if (auto spot_light = Object::instance_cast<SpotLightComponent>(light))
			{
				auto dir           = spot_light->direction() * 2.f;
				float outer_radius = 2.f * Math::tan(spot_light->outer_cone_angle());
				float inner_radius = 2.f * Math::tan(spot_light->inner_cone_angle());

				auto location = spot_light->world_transform().location + dir;
				lines.add_cone(location, -dir, outer_radius, {255, 255, 0, 255}, 0, 3.f);
				lines.add_cone(location, -dir, inner_radius, {255, 255, 0, 255}, 0, 3.f);
			}
			else if (auto point_light = Object::instance_cast<PointLightComponent>(light))
			{
				auto& location = point_light->world_transform().location;
				lines.add_sphere(location, point_light->attenuation_radius(), {255, 255, 0, 255}, 0, 3.f);
			}
			else if (auto directional_light = Object::instance_cast<DirectionalLightComponent>(light))
			{
				auto& location = directional_light->world_transform().location;
				lines.add_arrow(location, directional_light->direction(), {255, 255, 0, 255}, 3.f);
			}
		}

		return *this;
	}
}// namespace Engine
