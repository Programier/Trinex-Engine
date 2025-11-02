#include <Core/etl/templates.hpp>
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
#include <Engine/settings.hpp>
#include <Graphics/material_bindings.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <algorithm>

namespace Engine
{
	static constexpr uint_t s_cascades_per_directional_light = 4;

	static void find_light_range(FrameVector<LightComponent*>& lights, uint_t light_type, LightRenderRanges::LightRange& range,
	                             uint32_t search_offset = 0)
	{
		if (lights.size() <= search_offset)
		{
			range.normal.start = range.normal.end = search_offset;
			range.shadowed.start = range.shadowed.end = search_offset;
			return;
		}

		struct CompareType {
			bool operator()(const LightComponent* light, uint_t type) const { return light->light_type() < type; }
			bool operator()(uint_t type, const LightComponent* light) const { return light->light_type() > type; }
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
	    : Renderer(scene, view, mode), m_visible_primitives(scene->collect_visible_primitives(view.projview())),
	      m_visible_lights(scene->collect_visible_lights(view.projview())),
	      m_visible_post_processes(scene->collect_post_processes(view.camera_view().location))
	{
		static_sort_lights(m_visible_lights);
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
			case ViewMode::Wireframe:
			{
				graph->add_pass("Base Color Resolve")
				        .add_resource(base_color_target(), RHIAccess::TransferDst)
				        .add_resource(scene_color_ldr_target(), RHIAccess::TransferDst)
				        .add_func([this](RHIContext* ctx) { copy_base_color_to_scene_color(ctx); });
				break;
			}

			case ViewMode::WorldNormal:
			{
				graph->add_pass("World Normal Resolve")
				        .add_resource(normal_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) { copy_world_normal_to_scene_color(ctx); });
				break;
			}

			case ViewMode::Metalic:
			{
				graph->add_pass("Metalic Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) { copy_metalic_to_scene_color(ctx); });
				break;
			}

			case ViewMode::Roughness:
			{
				graph->add_pass("Roughness Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) { copy_roughness_to_scene_color(ctx); });
				break;
			}

			case ViewMode::Specular:
			{
				graph->add_pass("Specular Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) { copy_specular_to_scene_color(ctx); });
				break;
			}

			case ViewMode::Emissive:
			{
				graph->add_pass("Emissive Resolve")
				        .add_resource(scene_color_hdr_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) { copy_emissive_to_scene_color(ctx); });
				break;
			}

			case ViewMode::AO:
			{
				graph->add_pass("AO Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this](RHIContext* ctx) { copy_ambient_to_scene_color(ctx); });
				break;
			}

			default: break;
		}

		register_debug_lines();
	}

	static void render_octree(Scene::PrimitiveOctree::Node* node, const Frustum& frustum, BatchedLines& lines)
	{
		auto box = node->box();

		if (!frustum.intersects(node->box()))
			return;

		lines.add_box(box.min, box.max, {255, 0, 255, 255}, 3.f);

		for (byte i = 0; i < 8; i++)
		{
			auto child = node->child_at(i);

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
			render_octree(scene()->primitive_octree().root_node(), Frustum(scene_view().projview()), lines);
		}

		render_graph()
		        ->add_pass("Batched Primitives")
		        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this](RHIContext* ctx) {
			        if (!lines.is_empty())
			        {
				        ctx->bind_render_target1(scene_color_ldr_target()->as_rtv(), scene_depth_target()->as_dsv());
				        lines.flush(ctx, this);
			        }
		        });

		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(PointLightComponent* light, byte* shadow_data)
	{
		static const uint_t shadow_map_size = 1024;

		auto& transform = light->world_transform();

		CameraView view;
		view.location = transform.location();
		view.up       = transform.up_vector();
		view.forward  = transform.forward_vector();
		view.right    = transform.right_vector();

		view.projection_mode = CameraProjectionMode::Perspective;
		view.perspective.fov = 90.f;
		view.near            = 0.1f;
		view.far             = light->attenuation_radius();

		DepthCubeRenderer* renderer = FrameAllocator<DepthCubeRenderer>::allocate(1);
		new (renderer) DepthCubeRenderer(scene(), SceneView(view, {shadow_map_size, shadow_map_size}));
		add_child_renderer(renderer);

		PointLightShadowParameters* data = reinterpret_cast<PointLightShadowParameters*>(shadow_data);
		data->descriptor                 = renderer->cubemap()->as_srv()->descriptor();
		data->depth_bias                 = light->depth_bias() / static_cast<float>(shadow_map_size);
		data->slope_scale                = light->slope_scale() / static_cast<float>(shadow_map_size);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(SpotLightComponent* light, byte* shadow_data)
	{
		static const uint_t shadow_map_size = 1024;

		auto& transform = light->world_transform();

		CameraView view;
		view.location = transform.location();
		view.up       = transform.up_vector();
		view.forward  = transform.forward_vector();
		view.right    = transform.right_vector();

		view.projection_mode = CameraProjectionMode::Perspective;
		view.perspective.fov = glm::degrees(light->outer_cone_angle()) * 2.f;
		view.near            = 0.1f;
		view.far             = light->attenuation_radius();

		DepthRenderer* renderer = FrameAllocator<DepthRenderer>::allocate(1);
		new (renderer) DepthRenderer(scene(), SceneView(view, {shadow_map_size, shadow_map_size}));
		add_child_renderer(renderer);

		SpotLightShadowParameters* data = reinterpret_cast<SpotLightShadowParameters*>(shadow_data);
		data->descriptor                = renderer->scene_depth_target()->as_srv()->descriptor();
		data->projview                  = view.projection_matrix() * view.view_matrix();
		data->depth_bias                = light->depth_bias() / static_cast<float>(shadow_map_size);
		data->slope_scale               = light->slope_scale() / static_cast<float>(shadow_map_size);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(DirectionalLightComponent* light, byte* shadow_data)
	{
		static const uint_t shadow_map_size = 1024;
		const SceneView& view               = scene_view();

		DirectionalLightShadowParameters* data  = reinterpret_cast<DirectionalLightShadowParameters*>(shadow_data);
		DirectionalLightShadowCascade* cascades = reinterpret_cast<DirectionalLightShadowCascade*>(data + 1);

		const float camera_near = view.camera_view().near;
		const float camera_far  = view.camera_view().far;

		data->depth_bias  = light->depth_bias() / static_cast<float>(shadow_map_size);
		data->slope_scale = light->slope_scale() / static_cast<float>(shadow_map_size);
		data->splits      = {0.f, 0.f, 0.f, 0.f};

		const float shadow_distance = light->shadows_distance();

		for (uint_t cascade = 0; cascade < s_cascades_per_directional_light; ++cascade)
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

				for (uint_t i = 1; i < 8; i++)
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

			CameraView camera;
			camera.projection_mode = CameraProjectionMode::Orthographic;
			camera.up              = transform.up_vector();
			camera.forward         = transform.forward_vector();
			camera.right           = transform.right_vector();
			camera.location        = cascade_center;

			camera.ortho.left   = -cascade_radius;
			camera.ortho.right  = cascade_radius;
			camera.ortho.bottom = -cascade_radius;
			camera.ortho.top    = cascade_radius;

			camera.near = -100.f;
			camera.far  = 100.f;

			DepthRenderer* renderer = FrameAllocator<DepthRenderer>::allocate(1);
			new (renderer) DepthRenderer(scene(), SceneView(camera, {shadow_map_size, shadow_map_size}));
			add_child_renderer(renderer);

			cascades[cascade].projview   = camera.projection_matrix() * camera.view_matrix();
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
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
		        .add_resource(clusters_buffer(), RHIAccess::SRVGraphics)
		        .add_resource(lights_buffer(), RHIAccess::SRVGraphics)
		        .add_resource(shadow_buffer(), RHIAccess::SRVGraphics)
		        .add_func([this](RHIContext* ctx) { translucent_pass(ctx); });

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

		return *this;
	}

	DeferredRenderer& DeferredRenderer::geometry_pass(RHIContext* ctx)
	{
		RHIPolygonMode mode = view_mode() == ViewMode::Wireframe ? RHIPolygonMode::Line : RHIPolygonMode::Fill;
		ctx->polygon_mode(mode);

		ctx->bind_render_target4(base_color_target()->as_rtv(), normal_target()->as_rtv(), scene_color_hdr_target()->as_rtv(),
		                         msra_target()->as_rtv(), scene_depth_target()->as_dsv());
		render_visible_primitives(ctx, RenderPasses::Geometry::static_instance());

		ctx->polygon_mode(RHIPolygonMode::Fill);
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

		RHIPolygonMode mode = view_mode() == ViewMode::Wireframe ? RHIPolygonMode::Line : RHIPolygonMode::Fill;
		ctx->polygon_mode(mode);
		ctx->cull_mode(RHICullMode::Front);

		ctx->bind_render_target1(scene_color_hdr_target()->as_rtv(), scene_depth_target()->as_dsv());
		render_visible_primitives(ctx, RenderPasses::Translucent::static_instance(), &s_bindings);

		ctx->polygon_mode(RHIPolygonMode::Fill);
		ctx->cull_mode(RHICullMode::None);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::ambient_occlusion_pass(RHIContext* ctx)
	{
		RHITexturePool* pool = RHITexturePool::global_instance();

		Vector2u half_size     = scene_view().view_size() / 2u;
		Vector2f inv_half_size = 1.f / Vector2f(half_size);

		RHITexture* buffer1 = pool->request_surface(RHISurfaceFormat::R8, half_size);
		RHITexture* buffer2 = pool->request_surface(RHISurfaceFormat::R8, half_size);

		ctx->push_viewport(RHIViewport(half_size));
		ctx->push_scissor(RHIScissor(half_size));

		ctx->barrier(buffer1, RHIAccess::RTV);
		ctx->bind_render_target1(buffer1->as_rtv());

		// Render SSAO
		auto& ssao = m_post_process_params->ssao;
		Pipelines::SSAO::instance()->render(ctx, this, ssao.intensity, ssao.bias, ssao.power, ssao.radius, ssao.fade_out_distance,
		                                    ssao.fade_out_radius, ssao.samples);

		// Blur vertical
		ctx->barrier(buffer1, RHIAccess::SRVGraphics).barrier(buffer2, RHIAccess::RTV);
		ctx->bind_render_target1(buffer2->as_rtv());
		Pipelines::GaussianBlur::instance()->blur(ctx, buffer1->as_srv(), {0.f, inv_half_size.y}, 0.8, 2.f);

		// Blur horizontal
		ctx->barrier(buffer1, RHIAccess::RTV).barrier(buffer2, RHIAccess::SRVGraphics);
		ctx->bind_render_target1(buffer1->as_rtv());
		Pipelines::GaussianBlur::instance()->blur(ctx, buffer2->as_srv(), {inv_half_size.x, 0.f}, 0.8, 2.f);

		// ctx->bind_render_target1(msra_rtv);
		// ctx->write_mask(RHIColorComponent::A);
		// swizzle.a = Swizzle::R;
		// Pipelines::GaussianBlur::instance()->blur(ctx, buffer_srv, {inv_size.x, 0.f}, 0.8, 2.f, swizzle);
		// ctx->write_mask(RHIColorComponent::RGBA);


		pool->return_surface(buffer1);
		pool->return_surface(buffer2);

		ctx->pop_viewport();
		ctx->pop_scissor();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::global_illumination_pass(RHIContext* ctx)
	{
		RHISampler* sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();

		auto pipeline = Pipelines::AmbientLight::instance();
		ctx->bind_render_target1(scene_color_hdr_target()->as_rtv());

		ctx->depth_state(RHIDepthState(false, RHICompareFunc::Always, false));
		ctx->stencil_state(RHIStencilState(false));
		ctx->blending_state(RHIBlendingState::additive);

		ctx->bind_pipeline(pipeline->rhi_pipeline());
		ctx->bind_uniform_buffer(globals_uniform_buffer(), pipeline->scene_view->binding);
		ctx->update_scalar(&scene()->environment.ambient_color, pipeline->ambient_color);
		ctx->bind_srv(base_color_target()->as_srv(), pipeline->base_color->binding);
		ctx->bind_srv(msra_target()->as_srv(), pipeline->msra->binding);

		ctx->bind_sampler(sampler, pipeline->base_color->binding);
		ctx->bind_sampler(sampler, pipeline->msra->binding);

		ctx->draw(6, 0);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::deferred_lighting_pass(RHIContext* ctx)
	{
		RHISampler* sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();

		if (!m_visible_lights.empty())
		{
			auto pipeline = Pipelines::DeferredLighting::instance();

			ctx->depth_state(RHIDepthState(false, RHICompareFunc::Always, false));
			ctx->stencil_state(RHIStencilState(false));
			ctx->blending_state(RHIBlendingState::additive);

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

			ctx->draw(6, 0);
		}

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
		chain[0].texture = pool->request_surface(RHISurfaceFormat::RGBA16F, chain[0].size);

		RHITexture* const hdr = scene_color_hdr_target();

		trinex_rhi_push_stage(ctx, "Extract");

		ctx->blending_state(RHIBlendingState::opaque);
		ctx->viewport(RHIViewport(chain[0].size));

		ctx->barrier(chain[0].texture, RHIAccess::RTV);
		ctx->bind_render_target1(chain[0].texture->as_rtv());

		auto& bloom = m_post_process_params->bloom;
		Pipelines::BloomExtract::instance()->extract(ctx, hdr->as_srv(), bloom.threshold, bloom.knee, bloom.clamp);

		trinex_rhi_pop_stage(ctx);

		// Generate chain
		trinex_rhi_push_stage(ctx, "Downsample chain");
		{
			for (int i = 1; i < 6; ++i)
			{
				chain[i].size    = chain[i - 1].size / 2u;
				chain[i].texture = pool->request_surface(RHISurfaceFormat::RGBA16F, chain[i].size);

				ctx->barrier(chain[i - 1].texture, RHIAccess::SRVGraphics);
				ctx->barrier(chain[i].texture, RHIAccess::RTV);

				ctx->viewport(RHIViewport(chain[i].size));
				ctx->bind_render_target1(chain[i].texture->as_rtv());
				Pipelines::BloomDownsample::instance()->downsample(ctx, chain[i - 1].texture->as_srv());
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

				ctx->viewport(RHIViewport(chain[current].size));
				ctx->bind_render_target1(chain[current].texture->as_rtv());

				float fade = Math::lerp(bloom.fade_base, bloom.fade_max, static_cast<float>(current) / 5.f);
				Pipelines::BloomUpsample::instance()->upsample(ctx, chain[next].texture->as_srv(), fade);
			}

			trinex_rhi_pop_stage(ctx);

			trinex_rhi_push_stage(ctx, "Apply");

			ctx->barrier(chain[0].texture, RHIAccess::SRVGraphics);
			ctx->barrier(hdr, RHIAccess::RTV);

			ctx->viewport(RHIViewport(size));
			ctx->bind_render_target1(hdr->as_rtv());

			ctx->write_mask(RHIColorComponent::RGB);
			ctx->blending_state(RHIBlendingState::add);
			Pipelines::BloomUpsample::instance()->upsample(ctx, chain[0].texture->as_srv(), bloom.intensity);
			ctx->write_mask(RHIColorComponent::RGBA);

			trinex_rhi_pop_stage(ctx);
		}

		ctx->viewport(RHIViewport(size));
		for (int i = 0; i < 6; ++i) pool->return_surface(chain[i].texture);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_base_color_to_scene_color(RHIContext* ctx)
	{
		auto src = base_color_target();
		auto dst = scene_color_ldr_target();

		RHITextureRegion region(scene_view().view_size());
		ctx->copy_texture_to_texture(src, region, dst, region);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_world_normal_to_scene_color(RHIContext* ctx)
	{
		ctx->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = normal_target()->as_srv();
		Pipelines::Blit2D::instance()->blit(ctx, src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_metalic_to_scene_color(RHIContext* ctx)
	{
		ctx->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		Pipelines::Blit2D::instance()->blit(ctx, src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::R, Swizzle::R, Swizzle::R, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_specular_to_scene_color(RHIContext* ctx)
	{
		ctx->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		RHIRect rect(scene_view().view_size());
		Pipelines::Blit2D::instance()->blit(ctx, src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::G, Swizzle::G, Swizzle::G, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_roughness_to_scene_color(RHIContext* ctx)
	{
		ctx->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		RHIRect rect(scene_view().view_size());
		Pipelines::Blit2D::instance()->blit(ctx, src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::B, Swizzle::B, Swizzle::B, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_emissive_to_scene_color(RHIContext* ctx)
	{
		auto src = scene_color_hdr_target();
		auto dst = scene_color_ldr_target();

		RHITextureRegion region(scene_view().view_size());
		ctx->copy_texture_to_texture(src, region, dst, region);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_ambient_to_scene_color(RHIContext* ctx)
	{
		ctx->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		RHIRect rect(scene_view().view_size());
		Pipelines::Blit2D::instance()->blit(ctx, src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::A, Swizzle::A, Swizzle::A, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::render_visible_primitives(RHIContext* ctx, RenderPass* pass, MaterialBindings* bindings)
	{
		const FrameVector<PrimitiveComponent*>& primitives = visible_primitives();

		for (PrimitiveComponent* primitive : primitives)
		{
			const Matrix4f* matrix = &primitive->world_transform().matrix();
			PrimitiveRenderingContext context(this, ctx, pass, matrix, bindings);
			primitive->render(&context);
		}

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
			static constexpr RHIBufferCreateFlags flags = RHIBufferCreateFlags::StructuredBuffer |
			                                              RHIBufferCreateFlags::ShaderResource |
			                                              RHIBufferCreateFlags::TransferDst;

			size_t size     = sizeof(LightRenderParameters) * m_visible_lights.size();
			m_lights_buffer = RHIBufferPool::global_instance()->request_transient_buffer(size, flags);

			if (!m_visible_lights.empty())
			{
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
					uint_t current = m_light_ranges->point.shadowed.start;
					uint_t end     = m_light_ranges->point.shadowed.end;
					uint_t address = 0;

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

				rhi->context()->barrier(m_lights_buffer, RHIAccess::TransferDst);
				rhi->context()->update_buffer(m_lights_buffer, 0, size, reinterpret_cast<byte*>(parameters));
			}
		}
		return m_lights_buffer;
	}

	RHIBuffer* DeferredRenderer::shadow_buffer()
	{
		if (m_shadow_buffer == nullptr)
		{
			StackByteAllocator::Mark mark;

			uint32_t point_lights       = m_light_ranges->point.shadowed.end - m_light_ranges->point.shadowed.start;
			uint32_t spot_lights        = m_light_ranges->spot.shadowed.end - m_light_ranges->spot.shadowed.start;
			uint32_t directional_lights = m_light_ranges->directional.shadowed.end - m_light_ranges->directional.shadowed.start;

			size_t buffer_size =
			        point_lights * sizeof(PointLightShadowParameters) +//
			        spot_lights * sizeof(SpotLightShadowParameters) +  //
			        directional_lights * (sizeof(DirectionalLightShadowParameters) +
			                              s_cascades_per_directional_light * sizeof(DirectionalLightShadowCascade));//

			byte* buffer       = StackByteAllocator::allocate(buffer_size);
			byte* current_data = buffer;

			uint_t current = m_light_ranges->point.shadowed.start;
			uint_t end     = m_light_ranges->point.shadowed.end;

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

			auto pool       = RHIBufferPool::global_instance();
			m_shadow_buffer = pool->request_transient_buffer(buffer_size, RHIBufferCreateFlags::ByteAddressBuffer |
			                                                                      RHIBufferCreateFlags::ShaderResource |
			                                                                      RHIBufferCreateFlags::TransferDst);
			if (buffer_size > 0)
			{
				rhi->context()->update_buffer(m_shadow_buffer, 0, buffer_size, buffer);
			}
		}

		return m_shadow_buffer;
	}
}// namespace Engine
