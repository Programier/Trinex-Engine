#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/light_parameters.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	DeferredRenderer::DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode)
	    : Renderer(scene, view, mode), m_visible_primitives(scene->collect_visible_primitives(view.camera_view())),
	      m_visible_lights(scene->collect_visible_lights(view.camera_view()))
	{
		auto graph = render_graph();

		graph->add_output(scene_color_target());

		graph->add_pass(RenderGraph::Pass::Graphics, "Geometry Pass")
		        .add_resource(base_color_target(), RHIAccess::RTV)
		        .add_resource(normal_target(), RHIAccess::RTV)
		        .add_resource(emissive_target(), RHIAccess::RTV)
		        .add_resource(msra_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this]() { geometry_pass(); });


		if (mode == ViewMode::Lit)
		{
			graph->add_pass(RenderGraph::Pass::Graphics, "Lighting Pass")
			        .add_resource(base_color_target(), RHIAccess::SRVGraphics)
			        .add_resource(normal_target(), RHIAccess::SRVGraphics)
			        .add_resource(emissive_target(), RHIAccess::SRVGraphics)
			        .add_resource(msra_target(), RHIAccess::SRVGraphics)
			        .add_resource(scene_depth_target(), RHIAccess::SRVGraphics)
			        .add_resource(scene_color_target(), RHIAccess::RTV)
			        .add_func([this]() { deferred_lighting_pass(); });
		}
		else if (mode == ViewMode::Unlit)
		{
			graph->add_pass(RenderGraph::Pass::Graphics, "Base Color Resolve")
			        .add_resource(base_color_target(), RHIAccess::CopySrc)
			        .add_resource(scene_color_target(), RHIAccess::CopyDst)
			        .add_func([this]() { copy_base_color_to_scene_color(); });
		}
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
		const bool is_shadows_enabled = component->proxy()->is_shadows_enabled();

		if (is_shadows_enabled)
		{
			switch (component->light_type())
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

	DeferredRenderer& DeferredRenderer::deferred_lighting_pass()
	{
		RHI_Sampler* sampler = Sampler(RHISamplerFilter::Point).rhi_sampler();

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

		auto& lights = visible_lights();

		for (LightComponent* light : lights)
		{
			auto pipeline = static_find_light_pipeline(light);

			if (pipeline == nullptr)
				continue;

			LightRenderParameters parameters;
			light->proxy()->render_parameters(parameters);

			pipeline->rhi_bind();

			rhi->bind_srv(base_color_target()->as_srv(), pipeline->base_color_texture->binding);
			rhi->bind_srv(normal_target()->as_srv(), pipeline->normal_texture->binding);
			//rhi->bind_srv(emissive_target()->as_srv(), pipeline->emissive_texture->binding);
			rhi->bind_srv(msra_target()->as_srv(), pipeline->msra_texture->binding);
			rhi->bind_srv(scene_depth_target()->as_srv(), pipeline->depth_texture->binding);

			rhi->bind_sampler(sampler, pipeline->base_color_texture->binding);
			rhi->bind_sampler(sampler, pipeline->normal_texture->binding);
			//rhi->bind_sampler(sampler, pipeline->emissive_texture->binding);
			rhi->bind_sampler(sampler, pipeline->msra_texture->binding);
			rhi->bind_sampler(sampler, pipeline->depth_texture->binding);

			rhi->bind_uniform_buffer(globals_uniform_buffer(), pipeline->scene_view->binding);
			rhi->update_scalar_parameter(&parameters, pipeline->parameters);
			rhi->draw(6, 0);
		}

		return *this;
	}

	DeferredRenderer& DeferredRenderer::copy_base_color_to_scene_color()
	{
		auto src = base_color_target()->as_rtv();
		auto dst = scene_color_target()->as_rtv();

		RHIRect rect(scene_view().view_size());
		dst->blit(src, rect, rect, RHISamplerFilter::Point);
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
			        .add_resource(scene_color_target(), RHIAccess::RTV)
			        .add_resource(scene_depth_target(), RHIAccess::DSV)
			        .add_func([this]() {
				        rhi->bind_render_target1(scene_color_target()->as_rtv(), scene_depth_target()->as_dsv());
				        lines.flush(this);
			        });
		}

		Renderer::render();
		return *this;
	}
}// namespace Engine
