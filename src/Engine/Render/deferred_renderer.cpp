#include <Core/etl/templates.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/post_process_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/depth_renderer.hpp>
#include <Engine/Render/light_parameters.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/post_process_parameters.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>

namespace Engine
{
	DeferredRenderer::DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode)
	    : Renderer(scene, view, mode), m_visible_primitives(scene->collect_visible_primitives(view.camera_view())),
	      m_visible_lights(scene->collect_visible_lights(view.camera_view())),
	      m_visible_post_processes(scene->collect_post_processes(view.camera_view().location))
	{
		auto graph = render_graph();

		graph->add_output(scene_color_ldr_target());

		PostProcessParameters* post_process_params = FrameAllocator<PostProcessParameters>::allocate(1);
		new (post_process_params) PostProcessParameters();

		for (PostProcessComponent* post_process : visible_post_processes())
		{
			post_process_params->blend(post_process->parameters(), post_process->blend_weight());
		}

		graph->add_pass(RenderGraph::Pass::Graphics, "Geometry Pass")
		        .add_resource(base_color_target(), RHIAccess::RTV)
		        .add_resource(normal_target(), RHIAccess::RTV)
		        .add_resource(emissive_target(), RHIAccess::RTV)
		        .add_resource(msra_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this]() { geometry_pass(); });

		if (post_process_params->ssao.enabled)
		{
			graph->add_pass(RenderGraph::Pass::Graphics, "Ambient Occlusion")
			        .add_resource(msra_target(), RHIAccess::RTV)
			        .add_resource(normal_target(), RHIAccess::SRVGraphics)
			        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
			        .add_func([this, post_process_params]() { ambient_occlusion_pass(post_process_params); });
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
				graph->add_pass(RenderGraph::Pass::Graphics, "Base Color Resolve")
				        .add_resource(base_color_target(), RHIAccess::CopySrc)
				        .add_resource(scene_color_ldr_target(), RHIAccess::CopyDst)
				        .add_func([this]() { copy_base_color_to_scene_color(); });
				break;
			}

			case ViewMode::WorldNormal:
			{
				graph->add_pass(RenderGraph::Pass::Graphics, "World Normal Resolve")
				        .add_resource(normal_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this]() { copy_world_normal_to_scene_color(); });
				break;
			}

			case ViewMode::Metalic:
			{
				graph->add_pass(RenderGraph::Pass::Graphics, "Metalic Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this]() { copy_metalic_to_scene_color(); });
				break;
			}

			case ViewMode::Roughness:
			{
				graph->add_pass(RenderGraph::Pass::Graphics, "Roughness Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this]() { copy_roughness_to_scene_color(); });
				break;
			}

			case ViewMode::Specular:
			{
				graph->add_pass(RenderGraph::Pass::Graphics, "Specular Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this]() { copy_specular_to_scene_color(); });
				break;
			}

			case ViewMode::AO:
			{
				graph->add_pass(RenderGraph::Pass::Graphics, "AO Resolve")
				        .add_resource(msra_target(), RHIAccess::SRVGraphics)
				        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
				        .add_func([this]() { copy_ambient_to_scene_color(); });
				break;
			}

			default: break;
		}
	}

	DeferredRenderer& DeferredRenderer::register_shadow_light(SpotLightComponent* light, uint_t index)
	{
		Vector2u shadow_map_size = {512, 512};
		auto proxy               = light->proxy();

		auto& transform = proxy->world_transform();

		CameraView view;
		view.location       = transform.location();
		view.rotation       = transform.quaternion();
		view.up_vector      = transform.up_vector();
		view.forward_vector = transform.forward_vector();
		view.right_vector   = transform.right_vector();

		view.projection_mode = CameraProjectionMode::Perspective;
		view.aspect_ratio    = 1.f;
		view.near_clip_plane = scene_view().camera_view().near_clip_plane;
		view.far_clip_plane  = proxy->attenuation_radius();
		view.fov             = glm::degrees(proxy->outer_cone_angle()) * 2.f;

		DepthRenderer* renderer = FrameAllocator<DepthRenderer>::allocate(1);
		new (renderer) DepthRenderer(scene(), SceneView(view, shadow_map_size));
		add_child_renderer(renderer);

		m_shadow_maps[index]        = renderer->scene_depth_target();
		m_shadow_projections[index] = view.projection_matrix() * view.view_matrix();
		return *this;
	}

	DeferredRenderer& DeferredRenderer::register_lit_mode_passes()
	{
		auto graph = render_graph();

		graph->add_pass(RenderGraph::Pass::Graphics, "Lighting Pass")
		        .add_resource(base_color_target(), RHIAccess::SRVGraphics)
		        .add_resource(normal_target(), RHIAccess::SRVGraphics)
		        .add_resource(emissive_target(), RHIAccess::SRVGraphics)
		        .add_resource(msra_target(), RHIAccess::SRVGraphics)
		        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
		        .add_resource(scene_color_target(), RHIAccess::RTV)
		        .add_func([this]() { deferred_lighting_pass(); });


		m_shadow_maps.resize(m_visible_lights.size(), nullptr);
		m_shadow_projections.resize(m_visible_lights.size(), Matrix4f(1.f));

		for (size_t i = 0, count = m_visible_lights.size(); i < count; ++i)
		{
			auto component = m_visible_lights[i];
			auto light     = component->proxy();

			if (light->is_shadows_enabled())
			{
				if (light->light_type() == LightComponent::Spot)
				{
					register_shadow_light(static_cast<SpotLightComponent*>(component), i);
				}
			}
		}

		if (Settings::Rendering::enable_hdr)
		{
			// Tonemapping
			graph->add_pass(RenderGraph::Pass::Graphics, "Tonemapping")
			        .add_resource(scene_color_hdr_target(), RHIAccess::SRVGraphics)
			        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
			        .add_func([this]() { Pipelines::TonemappingACES::instance()->apply(this); });
		}

		return *this;
	}

	DeferredRenderer& DeferredRenderer::geometry_pass()
	{
		rhi->bind_render_target4(base_color_target()->as_rtv(), normal_target()->as_rtv(), emissive_target()->as_rtv(),
		                         msra_target()->as_rtv(), scene_depth_target()->as_dsv());
		render_visible_primitives(RenderPasses::Geometry::static_instance());
		return *this;
	}

	Pipelines::DeferredLightPipeline* DeferredRenderer::static_find_light_pipeline(LightComponent* component)
	{
		auto proxy                    = component->proxy();
		const bool is_shadows_enabled = proxy->is_shadows_enabled();

		if (is_shadows_enabled)
		{
			switch (proxy->light_type())
			{
				case LightComponent::Point: return Pipelines::DeferredPointLightShadowed::instance();
				case LightComponent::Spot: return Pipelines::DeferredSpotLightShadowed::instance();
				case LightComponent::Directional: return Pipelines::DeferredDirectionalLightShadowed::instance();
				default: return nullptr;
			}
		}
		else
		{
			switch (component->light_type())
			{
				case LightComponent::Point: return Pipelines::DeferredPointLight::instance();
				case LightComponent::Spot: return Pipelines::DeferredSpotLight::instance();
				case LightComponent::Directional: return Pipelines::DeferredDirectionalLight::instance();
				default: return nullptr;
			}
		}
	}

	DeferredRenderer& DeferredRenderer::ambient_occlusion_pass(PostProcessParameters* params)
	{
		RHISurfacePool* pool = RHISurfacePool::global_instance();

		Vector2f inv_size = 1.f / Vector2f(scene_view().view_size());

		RHITexture* buffer = pool->request_surface(RHISurfaceFormat::R8, scene_view().view_size());

		RHIRenderTargetView* buffer_rtv   = buffer->as_rtv();
		RHIShaderResourceView* buffer_srv = buffer->as_srv();

		RHIRenderTargetView* msra_rtv   = msra_target()->as_rtv();
		RHIShaderResourceView* msra_srv = msra_target()->as_srv();

		rhi->bind_render_target1(msra_target()->as_rtv());

		// Render SSAO
		Pipelines::SSAO::instance()->render(this, params->ssao.intensity, params->ssao.bias, params->ssao.power,
		                                    params->ssao.radius, params->ssao.fade_out_distance, params->ssao.fade_out_radius,
		                                    params->ssao.samples);
		Swizzle swizzle;

		// Blur vertical
		rhi->bind_render_target1(buffer_rtv);
		swizzle.r = Swizzle::A;
		Pipelines::GaussianBlur::instance()->blur(msra_srv, {0, 0}, inv_size, {1.f, 0.f}, 0.8, 2.f, swizzle);

		// Blur horizontal
		rhi->bind_render_target1(msra_rtv);
		rhi->write_mask(RHIColorComponent::A);
		swizzle.a = Swizzle::R;
		Pipelines::GaussianBlur::instance()->blur(buffer_srv, {0, 0}, inv_size, {1.f, 0.f}, 0.8, 2.f, swizzle);
		rhi->write_mask(RHIColorComponent::RGBA);

		pool->return_surface(buffer);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::deferred_lighting_pass()
	{
		RHISampler* sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();

		auto pipeline = Pipelines::AmbientLight::instance();
		rhi->bind_render_target1(scene_color_target()->as_rtv());

		pipeline->rhi_bind();

		rhi->bind_uniform_buffer(globals_uniform_buffer(), pipeline->scene_view->binding);
		rhi->update_scalar_parameter(&scene()->environment.ambient_color, pipeline->ambient_color);
		rhi->bind_srv(base_color_target()->as_srv(), pipeline->base_color->binding);
		rhi->bind_srv(msra_target()->as_srv(), pipeline->msra->binding);

		rhi->bind_sampler(sampler, pipeline->base_color->binding);
		rhi->bind_sampler(sampler, pipeline->msra->binding);

		rhi->draw(6, 0);

		for (size_t i = 0, count = m_visible_lights.size(); i < count; ++i)
		{
			auto component = m_visible_lights[i];
			auto pipeline  = static_find_light_pipeline(component);

			if (pipeline == nullptr)
				continue;

			auto light = component->proxy();

			LightRenderParameters parameters;
			light->render_parameters(parameters);

			pipeline->rhi_bind();

			rhi->bind_srv(base_color_target()->as_srv(), pipeline->base_color_texture->binding);
			rhi->bind_srv(normal_target()->as_srv(), pipeline->normal_texture->binding);
			rhi->bind_srv(msra_target()->as_srv(), pipeline->msra_texture->binding);
			rhi->bind_srv(scene_depth_target()->as_srv(), pipeline->depth_texture->binding);

			rhi->bind_sampler(sampler, pipeline->base_color_texture->binding);
			rhi->bind_sampler(sampler, pipeline->normal_texture->binding);
			rhi->bind_sampler(sampler, pipeline->msra_texture->binding);
			rhi->bind_sampler(sampler, pipeline->depth_texture->binding);

			rhi->bind_uniform_buffer(globals_uniform_buffer(), pipeline->scene_view->binding);
			rhi->update_scalar_parameter(&parameters, pipeline->parameters);

			if (light->is_shadows_enabled())
			{
				rhi->bind_srv(m_shadow_maps[i]->as_srv(), pipeline->shadow_map->binding);
				rhi->bind_sampler(RHIShadowSampler::static_sampler(), pipeline->shadow_map->binding);
				rhi->update_scalar_parameter(&m_shadow_projections[i], pipeline->shadow_projview);
			}

			rhi->draw(6, 0);
		}

		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_base_color_to_scene_color()
	{
		auto src = base_color_target();
		auto dst = scene_color_ldr_target();

		RHITextureRegion region(scene_view().view_size());
		rhi->copy_texture_to_texture(src, region, dst, region);
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_world_normal_to_scene_color()
	{
		rhi->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = normal_target()->as_srv();
		Pipelines::Blit2D::instance()->blit(src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_metalic_to_scene_color()
	{
		rhi->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		Pipelines::Blit2D::instance()->blit(src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::R, Swizzle::R, Swizzle::R, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_specular_to_scene_color()
	{
		rhi->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		RHIRect rect(scene_view().view_size());
		Pipelines::Blit2D::instance()->blit(src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::G, Swizzle::G, Swizzle::G, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_roughness_to_scene_color()
	{
		rhi->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		RHIRect rect(scene_view().view_size());
		Pipelines::Blit2D::instance()->blit(src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::B, Swizzle::B, Swizzle::B, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_ambient_to_scene_color()
	{
		rhi->bind_render_target1(scene_color_ldr_target()->as_rtv());
		auto src = msra_target()->as_srv();

		RHIRect rect(scene_view().view_size());
		Pipelines::Blit2D::instance()->blit(src, {0.f, 0.f}, 1.f / Vector2f(scene_view().view_size()),
		                                    {Swizzle::A, Swizzle::A, Swizzle::A, Swizzle::One});
		return *this;
	}

	DeferredRenderer& DeferredRenderer::render_visible_primitives(RenderPass* pass)
	{
		const FrameVector<PrimitiveComponent*>& primitives = visible_primitives();

		for (PrimitiveComponent* primitive : primitives)
		{
			render_primitive(pass, primitive);
		}

		return *this;
	}

	DeferredRenderer& DeferredRenderer::render()
	{
		if (!lines.is_empty())
		{
			render_graph()
			        ->add_pass(RenderGraph::Pass::Graphics, "Batched Primitives")
			        .add_resource(scene_color_ldr_target(), RHIAccess::RTV)
			        .add_resource(scene_depth_target(), RHIAccess::DSV)
			        .add_func([this]() {
				        rhi->bind_render_target1(scene_color_ldr_target()->as_rtv(), scene_depth_target()->as_dsv());
				        lines.flush(this);
			        });
		}

		Renderer::render();
		return *this;
	}
}// namespace Engine
