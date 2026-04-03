#include <Core/etl/templates.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/post_process_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/depth_renderer.hpp>
#include <Engine/Render/lighting.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/post_process_parameters.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_view_state.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material_bindings.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <algorithm>

namespace Trinex
{
	static constexpr u32 s_cascades_per_directional_light = 4;

	static void find_light_range(FrameVector<LightComponent*>& lights, u32 light_type, LightRenderRanges::LightRange& range,
	                             u32 search_offset = 0)
	{
		if (lights.size() <= search_offset)
		{
			range.normal.start = range.normal.end = search_offset;
			range.shadowed.start = range.shadowed.end = search_offset;
			return;
		}

		struct CompareType {
			bool operator()(const LightComponent* light, u32 type) const { return light->light_type() < type; }
			bool operator()(u32 type, const LightComponent* light) const { return light->light_type() > type; }
		};

		struct CompareShadows {
			bool operator()(const LightComponent* light, bool) const { return !light->is_shadows_enabled(); }
			bool operator()(bool, const LightComponent* light) const { return light->is_shadows_enabled(); }
		};

		auto lights_range = std::equal_range(lights.begin() + search_offset, lights.end(), light_type, CompareType());
		auto shadows      = std::lower_bound(lights_range.first, lights_range.second, true, CompareShadows());

		auto start = lights.begin();

		range.normal.start   = lights_range.first - start;
		range.normal.end     = shadows - start;
		range.shadowed.start = range.normal.end;
		range.shadowed.end   = lights_range.second - start;
	}

	DeferredRenderer::DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode)
	    : Renderer(scene, view, mode), m_visible_primitives(scene->collect_visible_primitives(view.camera_view().projview)),
	      m_visible_lights(scene->collect_visible_lights(view.camera_view().projview)),
	      m_visible_post_processes(scene->collect_post_processes(view.camera_view().location()))
	{
		sort_lights(m_visible_lights);
		m_light_ranges = FrameAllocator<LightRenderRanges>::allocate(1);

		find_light_range(m_visible_lights, LightComponent::Point, m_light_ranges->point);
		find_light_range(m_visible_lights, LightComponent::Spot, m_light_ranges->spot, m_light_ranges->point.shadowed.end);
		find_light_range(m_visible_lights, LightComponent::Directional, m_light_ranges->directional,
		                 m_light_ranges->spot.shadowed.end);

		auto graph = render_graph();

		m_post_process_params = trx_frame_new PostProcessParameters();

		for (PostProcessComponent* post_process : visible_post_processes())
		{
			m_post_process_params->blend(post_process->parameters(), post_process->blend_weight());
		}

		graph->add_pass("Geometry Pass")
		        .add_resource(base_color_target(), RHIAccess::RTV)
		        .add_resource(normal_target(), RHIAccess::RTV)
		        .add_resource(scene_color_hdr_target(), RHIAccess::RTV)
		        .add_resource(msra_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this](RHIContext* ctx) { geometry_pass(ctx); });

		graph->add_pass("Velocity Pass").add_resource(velocity_target(), RHIAccess::RTV).add_func([this](RHIContext* ctx) {
			velocity_pass(ctx);
		});

		if (m_post_process_params->ssao.enabled)
		{
			graph->add_pass("Ambient Occlusion")
			        .add_resource(msra_target(), RHIAccess::RTV)
			        .add_resource(normal_target(), RHIAccess::SRVGraphics)
			        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
			        .add_func([this](RHIContext* ctx) { ambient_occlusion_pass(ctx); });
		}

		switch (mode)
		{
			case ViewMode::Lit:
			{
				register_lit_mode_passes();
				break;
			}

			case ViewMode::Unlit:
			{
				graph->add_pass("Base Color Resolve")
				        .add_resource(base_color_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, base_color_target(), {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Wireframe:
			{
				graph->add_pass("Wireframe Rendering")
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_resource(scene_depth_target(), RHIAccess::DSV)
				        .add_func([this](RHIContext* ctx) { wireframe_pass(ctx); });
				break;
			}

			case ViewMode::WorldNormal:
			{
				graph->add_pass("World Normal Resolve")
				        .add_resource(normal_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, normal_target(), {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Emissive:
			{
				graph->add_pass("Emissive Resolve")
				        .add_resource(scene_color_hdr_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, scene_color_hdr_target(),
					                            {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Metalic:
			{
				graph->add_pass("Metalic Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, msra_target(), {Swizzle::R, Swizzle::R, Swizzle::R, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Specular:
			{
				graph->add_pass("Specular Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, msra_target(), {Swizzle::G, Swizzle::G, Swizzle::G, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Roughness:
			{
				graph->add_pass("Roughness Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, msra_target(), {Swizzle::B, Swizzle::B, Swizzle::B, Swizzle::One});
				        });
				break;
			}

			case ViewMode::AO:
			{
				graph->add_pass("AO Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, msra_target(), {Swizzle::A, Swizzle::A, Swizzle::A, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Velocity:
			{
				graph->add_pass("Velocity Resolve")
				        .add_resource(velocity_target(), RHIAccess::TransferSrc)
				        .add_resource(scene_color_ldr_target(), RHIAccess::TransferDst)
				        .add_func([this](RHIContext* ctx) {
					        copy_to_scene_color(ctx, velocity_target(), {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One});
				        });
				break;
			}

			case ViewMode::Depth:
			{
				graph->add_pass("Depth Resolve")
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
				        .add_func([this](RHIContext* ctx) { copy_depth_to_scene_color(ctx); });
				break;
			}

			default: break;
		}

		if (mode == ViewMode::Lit)
		{
			if (m_post_process_params->bloom.enabled)
			{
				graph->add_pass("Bloom")
				        .add_resource(scene_color_hdr_target(), RHIAccess::RTV, RHIAccess::SRVGraphics)
				        .add_func([this](RHIContext* ctx) { bloom_pass(ctx); });
			}

			// Tonemapping
			graph->add_pass("Tonemapping")
			        .add_resource(scene_color_hdr_target(), RHIAccess::SRVGraphics)
			        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
			        .add_func([this](RHIContext* ctx) { Pipelines::TonemappingACES::instance()->apply(ctx, this); });
		}

		register_debug_lines();
	}

	static void render_octree(Octree::Node* node, const Frustum& frustum, BatchedLines& lines)
	{
		auto box = node->box();

		if (!frustum.intersects(node->box()))
			return;

		lines.add_box(box.min, box.max, {255, 0, 255, 255}, 3.f);

		for (u8 i = 0; i < 8; i++)
		{
			auto child = node->child(i);

			if (child)
			{
				render_octree(child, frustum, lines);
			}
		}
	}

	DeferredRenderer& DeferredRenderer::register_debug_lines()
	{
		ShowFlags flags = scene_view().show_flags();

		if (flags & ShowFlags::PrimitiveBounds)
		{
			for (PrimitiveComponent* component : m_visible_primitives)
			{
				auto& bounds = component->bounding_box();
				lines.add_box(bounds.min - Vector3f(0.01f), bounds.max + Vector3f(0.01f), {255, 255, 0, 255}, 3.f);
			}
		}

		if (flags & ShowFlags::PrimitiveOctree)
		{
			render_octree(scene()->primitive_octree().root(), Frustum(scene_view().camera_view().projview), lines);
		}

		render_graph()
		        ->add_pass("Batched Primitives")
		        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
		        .add_untracked_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this](RHIContext* ctx) {
			        if (!lines.is_empty())
			        {
				        lines.flush(ctx, this);
			        }
		        });

		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(PointLightComponent* light, u8* shadow_data)
	{
		static const u32 shadow_map_size = 1024;

		auto& transform = light->world_transform();

		CameraView view = CameraView::static_perspective(transform.location, transform.forward_vector(), transform.up_vector(),
		                                                 Math::pi() / 2.f, 1.f, 0.1, light->attenuation_radius());

		DepthCubeRenderer* renderer = FrameAllocator<DepthCubeRenderer>::allocate(1);
		new (renderer) DepthCubeRenderer(scene(), SceneView(view, {shadow_map_size, shadow_map_size}));
		add_child_renderer(renderer);

		PointLightShadowParameters* data = reinterpret_cast<PointLightShadowParameters*>(shadow_data);
		data->descriptor                 = renderer->cubemap()->as_srv()->descriptor();
		data->depth_bias                 = light->depth_bias() / static_cast<float>(shadow_map_size);
		data->slope_scale                = light->slope_scale() / static_cast<float>(shadow_map_size);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(SpotLightComponent* light, u8* shadow_data)
	{
		static const u32 shadow_map_size = 1024;

		auto& transform = light->world_transform();

		CameraView view = CameraView::static_perspective(transform.location, transform.forward_vector(), transform.up_vector(),
		                                                 light->outer_cone_angle() * 2.f, 1.f, 0.1, light->attenuation_radius());

		DepthRenderer* renderer = FrameAllocator<DepthRenderer>::allocate(1);
		new (renderer) DepthRenderer(scene(), SceneView(view, {shadow_map_size, shadow_map_size}));
		add_child_renderer(renderer);

		SpotLightShadowParameters* data = reinterpret_cast<SpotLightShadowParameters*>(shadow_data);
		data->descriptor                = renderer->scene_depth_target()->as_srv()->descriptor();
		data->projview                  = view.projview;
		data->depth_bias                = light->depth_bias() / static_cast<float>(shadow_map_size);
		data->slope_scale               = light->slope_scale() / static_cast<float>(shadow_map_size);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(DirectionalLightComponent* light, u8* shadow_data)
	{
		static const u32 shadow_map_size = 1024;
		const SceneView& view            = scene_view();

		DirectionalLightShadowParameters* data  = reinterpret_cast<DirectionalLightShadowParameters*>(shadow_data);
		DirectionalLightShadowCascade* cascades = reinterpret_cast<DirectionalLightShadowCascade*>(data + 1);

		const float camera_near = view.camera_view().near;
		const float camera_far  = view.camera_view().far;

		data->depth_bias  = light->depth_bias() / static_cast<float>(shadow_map_size);
		data->slope_scale = light->slope_scale() / static_cast<float>(shadow_map_size);
		data->splits      = {0.f, 0.f, 0.f, 0.f};

		const float shadow_distance = light->shadows_distance();

		for (u32 cascade = 0; cascade < s_cascades_per_directional_light; ++cascade)
		{
			Vector3f cascade_center;
			float cascade_radius;

			{
				float near = Math::cascade_split(cascade, s_cascades_per_directional_light);
				float far  = Math::cascade_split(cascade + 1, s_cascades_per_directional_light);

				near = Math::unlinearize_depth(camera_near + shadow_distance * near, camera_near, camera_far);
				far  = Math::unlinearize_depth(camera_near + shadow_distance * far, camera_near, camera_far);

				data->splits[cascade] = far;

				const Vector3f screen_corners[] = {
				        {-1.f, -1.f, near}, {-1.f, 1.f, near}, {1.f, -1.f, near}, {1.f, 1.f, near},
				        {-1.f, -1.f, far},  {-1.f, 1.f, far},  {1.f, -1.f, far},  {1.f, 1.f, far},
				};

				Vector3f corner = view.screen_to_world(screen_corners[0]);

				Box3f box = Box3f(corner, corner);

				for (u32 i = 1; i < 8; i++)
				{
					corner  = view.screen_to_world(screen_corners[i]);
					box.min = Math::min(box.min, corner);
					box.max = Math::max(box.max, corner);
				}

				cascade_radius = Math::ceil(box.radius() * 16.0f) / 16.0f;
				cascade_center = box.center();

				float texels_per_unit = static_cast<float>(shadow_map_size) / (cascade_radius * 2.0f);
				cascade_center        = Math::floor(cascade_center * texels_per_unit) / texels_per_unit;
			}

			auto& transform = light->world_transform();

			CameraView camera =
			        CameraView::static_ortho(transform.location, transform.forward_vector(), transform.up_vector(),
			                                 -cascade_radius, cascade_radius, cascade_radius, -cascade_radius, -100, 100);

			DepthRenderer* renderer = FrameAllocator<DepthRenderer>::allocate(1);
			new (renderer) DepthRenderer(scene(), SceneView(camera, {shadow_map_size, shadow_map_size}));
			add_child_renderer(renderer);

			cascades[cascade].projview   = camera.projview;
			cascades[cascade].descriptor = renderer->scene_depth_target()->as_srv()->descriptor();
		}

		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_lit_mode_passes()
	{
		auto graph = render_graph();

		graph->add_pass("Light Culling")
		        .add_resource(clusters_buffer(), RHIAccess::UAVCompute)
		        .add_resource(lights_buffer(), RHIAccess::SRVCompute)
		        .add_func([this](RHIContext* ctx) { cull_lights(ctx); });

		graph->add_pass("Global Illumination")
		        .add_resource(scene_color_hdr_target(), RHIAccess::RTV)
		        .add_resource(base_color_target(), RHIAccess::SRVGraphics)
		        .add_resource(msra_target(), RHIAccess::SRVGraphics)
		        .add_func([this](RHIContext* ctx) { global_illumination_pass(ctx); });

		graph->add_pass("Lighting Pass")
		        .add_resource(base_color_target(), RHIAccess::SRVGraphics)
		        .add_resource(normal_target(), RHIAccess::SRVGraphics)
		        .add_resource(msra_target(), RHIAccess::SRVGraphics)
		        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
		        .add_resource(scene_color_hdr_target(), RHIAccess::RTV)
		        .add_resource(clusters_buffer(), RHIAccess::SRVGraphics)
		        .add_resource(lights_buffer(), RHIAccess::SRVGraphics)
		        .add_resource(shadow_buffer(), RHIAccess::SRVGraphics)
		        .add_func([this](RHIContext* ctx) { deferred_lighting_pass(ctx); });

		graph->add_pass("Translucent")
		        .add_resource(scene_color_hdr_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSVRead)
		        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
		        .add_resource(clusters_buffer(), RHIAccess::SRVGraphics)
		        .add_resource(lights_buffer(), RHIAccess::SRVGraphics)
		        .add_resource(shadow_buffer(), RHIAccess::SRVGraphics)
		        .add_func([this](RHIContext* ctx) { translucent_pass(ctx); });

		return *this;
	}

	DeferredRenderer& DeferredRenderer::wireframe_pass(RHIContext* ctx)
	{
		// RHITexture* depth = request_surface(RHISurfaceFormat::D16);

		// // Clear depth
		// ctx->barrier(depth, RHIAccess::TransferDst);
		// ctx->clear_dsv(depth->as_dsv());
		// ctx->barrier(depth, RHIAccess::DSV);

		// RHIRenderingInfo info = {scene_color_ldr_target()->as_rtv(), depth->as_dsv()};
		// info.flags            = RHIRenderingFlags::SecondaryBuffersOnly;

		// ctx->begin_rendering(info);
		// {
		// 	RHIContextInheritanceInfo inherit;
		// 	inherit.primary   = ctx;
		// 	inherit.colors[0] = scene_color_ldr_format();
		// 	inherit.depth     = RHISurfaceFormat::D16;
		// 	inherit.flags     = RHIContextInheritanceFlags::RenderPassContinue;

		// 	render_visible_primitives(ctx, RenderPasses::Wireframe::static_instance(), &inherit);
		// }
		// ctx->end_rendering();

		// return_surface(depth);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::geometry_pass(RHIContext* ctx)
	{
		RHIRenderingInfo info = {base_color_target()->as_rtv(), normal_target()->as_rtv(), scene_color_hdr_target()->as_rtv(),
		                         msra_target()->as_rtv(), scene_depth_target()->as_dsv()};
		info.flags            = RHIRenderingFlags::SecondaryBuffersOnly;

		ctx->begin_rendering(info);
		{
			RHIContextInheritanceInfo inherit;
			inherit.primary   = ctx;
			inherit.colors[0] = base_color_format();
			inherit.colors[1] = normal_format();
			inherit.colors[2] = scene_color_hdr_format();
			inherit.colors[3] = msra_format();
			inherit.depth     = scene_depth_format();
			inherit.flags     = RHIContextInheritanceFlags::RenderPassContinue;

			render_visible_primitives(ctx, RenderPasses::Geometry::static_instance(), &inherit);
		}
		ctx->end_rendering();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::velocity_pass(RHIContext* ctx)
	{
		ctx->begin_rendering(velocity_target()->as_rtv());
		{
			ctx->blending_state(RHIBlendingState::opaque());
			Pipelines::CameraVelocity::instance()->render(ctx, this);
		}
		ctx->end_rendering();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::translucent_pass(RHIContext* ctx)
	{
		static MaterialBindings s_bindings;
		static MaterialBindings::Binding* s_shadow_sampler = s_bindings.find_or_create("shadow_sampler");
		static MaterialBindings::Binding* s_clusters       = s_bindings.find_or_create("clusters");
		static MaterialBindings::Binding* s_lights         = s_bindings.find_or_create("lights");
		static MaterialBindings::Binding* s_shadows        = s_bindings.find_or_create("shadows");
		static MaterialBindings::Binding* s_ranges         = s_bindings.find_or_create("ranges");

		MaterialBindings::MemoryBlock ranges_block;
		ranges_block.memory = m_light_ranges;
		ranges_block.size   = sizeof(*m_light_ranges);

		s_shadow_sampler->emplace<RHISampler*>(RHIShadowSampler::static_sampler());
		s_clusters->emplace<RHIShaderResourceView*>(clusters_buffer()->as_srv());
		s_lights->emplace<RHIShaderResourceView*>(lights_buffer()->as_srv());
		s_shadows->emplace<RHIShaderResourceView*>(shadow_buffer()->as_srv());
		s_ranges->emplace<MaterialBindings::MemoryBlock>(ranges_block);

		RHIRenderingInfo info = {scene_color_hdr_target()->as_rtv(), scene_depth_target()->as_dsv()};
		info.flags            = RHIRenderingFlags::SecondaryBuffersOnly;

		ctx->begin_rendering(info);
		{
			RHIContextInheritanceInfo inherit;
			inherit.primary   = ctx;
			inherit.colors[0] = scene_color_hdr_format();
			inherit.depth     = scene_depth_format();
			inherit.flags     = RHIContextInheritanceFlags::RenderPassContinue;

			render_visible_primitives(ctx, RenderPasses::Translucent::static_instance(), &inherit, &s_bindings);
		}
		ctx->end_rendering();

		return *this;
	}

	DeferredRenderer& DeferredRenderer::ambient_occlusion_pass(RHIContext* ctx)
	{
		RHITexturePool* pool = RHITexturePool::global_instance();

		Vector2u half_size     = scene_view().view_size() / 2u;
		Vector2f inv_half_size = 1.f / Vector2f(half_size);

		RHITexture* buffer1 = pool->request_surface(RHISurfaceFormat::R8, half_size);
		RHITexture* buffer2 = pool->request_surface(RHISurfaceFormat::R8, half_size);

		ctx->barrier(buffer1, RHIAccess::RTV);

		// Render SSAO
		ctx->begin_rendering(buffer1->as_rtv());
		{
			ctx->push_debug_stage("Calculate");
			auto& ssao = m_post_process_params->ssao;
			Pipelines::SSAO::instance()->render(ctx, this, ssao.intensity, ssao.bias, ssao.power, ssao.radius,
			                                    ssao.fade_out_distance, ssao.fade_out_radius, ssao.samples);

			ctx->pop_debug_stage();
		}
		ctx->end_rendering();

		ctx->push_debug_stage("Blur and Apply");

		// Blur vertical
		ctx->barrier(buffer1, RHIAccess::SRVGraphics).barrier(buffer2, RHIAccess::RTV);

		ctx->begin_rendering(buffer2->as_rtv());
		{
			Pipelines::GaussianBlur::blur(ctx, buffer1->as_srv(), {0.f, inv_half_size.y}, 0.8, 2.f);
		}
		ctx->end_rendering();

		// Blur horizontal
		ctx->barrier(buffer2, RHIAccess::SRVGraphics);

		ctx->begin_rendering(msra_target()->as_rtv());
		{
			ctx->blending_state(RHIBlendingState::multiply());

			Pipelines::GaussianBlur::blur(ctx, buffer2->as_srv(), {inv_half_size.x, 0.f}, 0.8, 2.f,
			                              {Swizzle::One, Swizzle::One, Swizzle::One, Swizzle::R});
		}
		ctx->end_rendering();

		ctx->pop_debug_stage();

		pool->return_surface(buffer1);
		pool->return_surface(buffer2);

		return *this;
	}

	DeferredRenderer& DeferredRenderer::global_illumination_pass(RHIContext* ctx)
	{
		RHISampler* sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();

		auto pipeline = Pipelines::AmbientLight::instance();

		ctx->begin_rendering(scene_color_hdr_target()->as_rtv());
		{
			ctx->depth_stencil_state(RHIDepthStencilState());
			ctx->blending_state(RHIBlendingState::additive());

			ctx->bind_pipeline(pipeline->rhi_pipeline());
			ctx->bind_uniform_buffer(globals_uniform_buffer(), pipeline->scene_view->binding);
			ctx->update_scalar(&scene()->environment.ambient_color, pipeline->ambient_color);
			ctx->bind_srv(base_color_target()->as_srv(), pipeline->base_color->binding);
			ctx->bind_srv(msra_target()->as_srv(), pipeline->msra->binding);

			ctx->bind_sampler(sampler, pipeline->base_color->binding);
			ctx->bind_sampler(sampler, pipeline->msra->binding);

			ctx->draw(RHITopology::TriangleList, 6, 0);
		}
		ctx->end_rendering();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::deferred_lighting_pass(RHIContext* ctx)
	{
		if (m_visible_lights.empty())
			return *this;

		ctx->begin_rendering(scene_color_hdr_target()->as_rtv());
		{
			RHISampler* sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();
			auto pipeline       = Pipelines::DeferredLighting::instance();

			ctx->depth_stencil_state(RHIDepthStencilState());
			ctx->blending_state(RHIBlendingState::additive());
			ctx->rasterizer_state(RHIRasterizerState());

			ctx->bind_pipeline(pipeline->rhi_pipeline());
			ctx->bind_srv(base_color_target()->as_srv(), pipeline->base_color_texture->binding);
			ctx->bind_srv(normal_target()->as_srv(), pipeline->normal_texture->binding);
			ctx->bind_srv(msra_target()->as_srv(), pipeline->msra_texture->binding);
			ctx->bind_srv(scene_depth_target()->as_srv(), pipeline->depth_texture->binding);

			ctx->bind_sampler(sampler, pipeline->screen_sampler->binding);
			ctx->bind_sampler(RHIShadowSampler::static_sampler(), pipeline->shadow_sampler->binding);

			ctx->bind_uniform_buffer(globals_uniform_buffer(), pipeline->scene_view->binding);
			ctx->bind_srv(clusters_buffer()->as_srv(), pipeline->clusters->binding);
			ctx->bind_srv(lights_buffer()->as_srv(), pipeline->lights->binding);
			ctx->bind_srv(shadow_buffer()->as_srv(), pipeline->shadows->binding);

			ctx->update_scalar(m_light_ranges, pipeline->ranges);

			ctx->draw(RHITopology::TriangleList, 6, 0);
		}
		ctx->end_rendering();

		return *this;
	}

	DeferredRenderer& DeferredRenderer::temporal_antialiasing_pass(RHIContext* ctx)
	{
		// scene_view().state()->resize(ctx, scene_view().view_size());
		// Pipelines::TAA::instance()->render(ctx, this);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::bloom_pass(RHIContext* ctx)
	{
		const Vector2u size = scene_view().view_size();
		auto pool           = RHITexturePool::global_instance();

		struct Chain {
			RHITexture* texture;
			Vector2u size;
		};

		Chain chain[6];
		chain[0].size    = size / 2u;
		chain[0].texture = pool->request_surface(RHISurfaceFormat::RGBA16F, chain[0].size, RHITextureFlags::ColorAttachment);

		RHITexture* const hdr = scene_color_hdr_target();

		trinex_rhi_push_stage(ctx, "Extract");

		auto& bloom = m_post_process_params->bloom;

		ctx->depth_stencil_state(RHIDepthStencilState());
		ctx->rasterizer_state(RHIRasterizerState());
		ctx->blending_state(RHIBlendingState::opaque());

		ctx->barrier(chain[0].texture, RHIAccess::RTV);

		ctx->begin_rendering(chain[0].texture->as_rtv());
		{
			auto& bloom = m_post_process_params->bloom;
			Pipelines::BloomExtract::instance()->extract(ctx, hdr->as_srv(), bloom.threshold, bloom.knee, bloom.clamp);
		}
		ctx->end_rendering();

		trinex_rhi_pop_stage(ctx);

		// Generate chain
		trinex_rhi_push_stage(ctx, "Downsample chain");
		{
			for (int i = 1; i < 6; ++i)
			{
				chain[i].size = chain[i - 1].size / 2u;
				chain[i].texture =
				        pool->request_surface(RHISurfaceFormat::RGBA16F, chain[i].size, RHITextureFlags::ColorAttachment);

				ctx->barrier(chain[i - 1].texture, RHIAccess::SRVGraphics);
				ctx->barrier(chain[i].texture, RHIAccess::RTV);

				ctx->begin_rendering(chain[i].texture->as_rtv());
				{
					Pipelines::BloomDownsample::instance()->downsample(ctx, chain[i - 1].texture->as_srv());
				}
				ctx->end_rendering();
			}
		}
		trinex_rhi_pop_stage(ctx);

		// Composite chain
		{
			trinex_rhi_push_stage(ctx, "Upsample chain");

			for (int current = 4; current >= 0; --current)
			{
				int next = current + 1;

				ctx->barrier(chain[next].texture, RHIAccess::SRVGraphics);
				ctx->barrier(chain[current].texture, RHIAccess::RTV);

				ctx->begin_rendering(chain[current].texture->as_rtv());
				{
					float fade = Math::lerp(bloom.fade_base, bloom.fade_max, static_cast<float>(current) / 5.f);
					Pipelines::BloomUpsample::instance()->upsample(ctx, chain[next].texture->as_srv(), fade);
				}
				ctx->end_rendering();
			}

			trinex_rhi_pop_stage(ctx);
			trinex_rhi_push_stage(ctx, "Apply");

			ctx->barrier(chain[0].texture, RHIAccess::SRVGraphics);
			ctx->barrier(hdr, RHIAccess::RTV);

			ctx->begin_rendering(hdr->as_rtv());
			{
				ctx->blending_state(RHIBlendingState::add(RHIColorComponent::RGB));
				Pipelines::BloomUpsample::instance()->upsample(ctx, chain[0].texture->as_srv(), bloom.intensity);
			}
			ctx->end_rendering();

			trinex_rhi_pop_stage(ctx);
		}

		for (int i = 0; i < 6; ++i) pool->return_surface(chain[i].texture);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_to_scene_color(RHIContext* ctx, RHITexture* src, const Swizzle& swizzle)
	{
		auto dst = scene_color_ldr_target();
		ctx->begin_rendering(dst->as_rtv());
		{
			ctx->blending_state(RHIBlendingState());
			Pipelines::Blit2D::instance()->blit(ctx, src->as_srv(), {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
			                                    swizzle);
		}
		ctx->end_rendering();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_depth_to_scene_color(RHIContext* ctx)
	{
		ctx->depth_stencil_state(RHIDepthStencilState());
		ctx->blending_state(RHIBlendingState());
		ctx->rasterizer_state(RHIRasterizerState());

		ctx->begin_rendering(RHIRenderingInfo(scene_color_ldr_target()->as_rtv()));
		{
			Pipelines::DepthView::instance()->render(ctx, this);
		}
		ctx->end_rendering();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::cull_lights(RHIContext* ctx)
	{
		Pipelines::ClusterLightCulling::instance()->cull(ctx, this, clusters_buffer(), lights_buffer(), *m_light_ranges);
		return *this;
	}

	RHIBuffer* DeferredRenderer::clusters_buffer()
	{
		if (m_clusters_buffer == nullptr)
		{
			m_clusters_buffer = Pipelines::ClusterInitialize::instance()->create_clusters_buffer();

			render_graph()
			        ->add_pass("Initialize Clusters")
			        .add_resource(m_clusters_buffer, RHIAccess::UAVCompute)
			        .add_func([this](RHIContext* ctx) {
				        Pipelines::ClusterInitialize::instance()->build(ctx, m_clusters_buffer, this);
			        });
		}
		return m_clusters_buffer;
	}

	RHIBuffer* DeferredRenderer::lights_buffer()
	{
		if (m_lights_buffer == nullptr)
		{
			StackByteAllocator::Mark mark;
			static constexpr RHIBufferFlags flags =
			        RHIBufferFlags::StructuredBuffer | RHIBufferFlags::ShaderResource | RHIBufferFlags::TransferDst;

			usize size      = sizeof(LightRenderParameters) * m_visible_lights.size();
			m_lights_buffer = RHIBufferPool::global_instance()->request_transient_buffer(size, flags);

			if (!m_visible_lights.empty())
			{
				auto pass = [this](RHIContext* ctx) {
					usize size = sizeof(LightRenderParameters) * m_visible_lights.size();

					LightRenderParameters* parameters = StackAllocator<LightRenderParameters>::allocate(m_visible_lights.size());
					{
						LightRenderParameters* current = parameters;

						for (LightComponent* light : m_visible_lights)
						{
							light->render_parameters(*(current++));
						}
					}

					// Update shadow buffer address for each light
					{
						u32 current = m_light_ranges->point.shadowed.start;
						u32 end     = m_light_ranges->point.shadowed.end;
						u32 address = 0;

						for (; current < end; ++current)
						{
							parameters[current].shadow_address = address;
							address += sizeof(PointLightShadowParameters);
						}

						current = m_light_ranges->spot.shadowed.start;
						end     = m_light_ranges->spot.shadowed.end;

						for (; current < end; ++current)
						{
							parameters[current].shadow_address = address;
							address += sizeof(SpotLightShadowParameters);
						}

						current = m_light_ranges->directional.shadowed.start;
						end     = m_light_ranges->directional.shadowed.end;

						for (; current < end; ++current)
						{
							parameters[current].shadow_address = address;
							address += sizeof(DirectionalLightShadowParameters) +
							           s_cascades_per_directional_light * sizeof(DirectionalLightShadowCascade);
						}
					}

					ctx->barrier(m_lights_buffer, RHIAccess::TransferDst);
					ctx->update(m_lights_buffer, parameters, {.size = size});
				};

				render_graph()
				        ->add_pass("Initialize Light Buffer")
				        .add_resource(m_lights_buffer, RHIAccess::TransferDst)
				        .add_func(pass);
			}
		}
		return m_lights_buffer;
	}

	RHIBuffer* DeferredRenderer::shadow_buffer()
	{
		if (m_shadow_buffer == nullptr)
		{
			StackByteAllocator::Mark mark;

			u32 point_lights       = m_light_ranges->point.shadowed.end - m_light_ranges->point.shadowed.start;
			u32 spot_lights        = m_light_ranges->spot.shadowed.end - m_light_ranges->spot.shadowed.start;
			u32 directional_lights = m_light_ranges->directional.shadowed.end - m_light_ranges->directional.shadowed.start;

			usize buffer_size = point_lights * sizeof(PointLightShadowParameters) +//
			                    spot_lights * sizeof(SpotLightShadowParameters) +  //
			                    directional_lights * (sizeof(DirectionalLightShadowParameters) +
			                                          s_cascades_per_directional_light * sizeof(DirectionalLightShadowCascade));//

			auto pool       = RHIBufferPool::global_instance();
			m_shadow_buffer = pool->request_transient_buffer(buffer_size, RHIBufferFlags::ByteAddressBuffer |
			                                                                      RHIBufferFlags::ShaderResource |
			                                                                      RHIBufferFlags::TransferDst);

			if (buffer_size == 0)
				return m_shadow_buffer;

			auto pass = [this, buffer_size](RHIContext* ctx) {
				u8* buffer       = StackByteAllocator::allocate(buffer_size);
				u8* current_data = buffer;

				u32 current = m_light_ranges->point.shadowed.start;
				u32 end     = m_light_ranges->point.shadowed.end;

				for (; current < end; ++current)
				{
					auto light = static_cast<PointLightComponent*>(m_visible_lights[current]);
					register_shadow_light(light, current_data);
					current_data += sizeof(PointLightShadowParameters);
				}

				current = m_light_ranges->spot.shadowed.start;
				end     = m_light_ranges->spot.shadowed.end;

				for (; current < end; ++current)
				{
					auto light = static_cast<SpotLightComponent*>(m_visible_lights[current]);
					register_shadow_light(light, current_data);
					current_data += sizeof(SpotLightShadowParameters);
				}

				current = m_light_ranges->directional.shadowed.start;
				end     = m_light_ranges->directional.shadowed.end;

				for (; current < end; ++current)
				{
					auto light = static_cast<DirectionalLightComponent*>(m_visible_lights[current]);
					register_shadow_light(light, current_data);

					current_data += sizeof(DirectionalLightShadowParameters) +
					                s_cascades_per_directional_light * sizeof(DirectionalLightShadowCascade);
				}

				ctx->update(m_shadow_buffer, buffer, {.size = buffer_size});
			};

			render_graph()
			        ->add_pass("Initialize Shadow Buffer")
			        .add_resource(m_shadow_buffer, RHIAccess::TransferDst)
			        .add_func(pass);
		}

		return m_shadow_buffer;
	}
}// namespace Trinex
