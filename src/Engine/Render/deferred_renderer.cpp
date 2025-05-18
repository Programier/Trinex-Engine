#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/deferred_renderer.hpp>
#include <Engine/Render/light_parameters.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/scene.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
	DeferredRenderer::DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode) : Renderer(scene, view, mode) {}

	DeferredRenderer& DeferredRenderer::render(RenderGraph::Graph& graph)
	{
		graph.add_pass(RenderGraph::Pass::Graphics, "Geometry Pass")
		        .add_resource(base_color_target(), RHIAccess::RTV)
		        .add_resource(normal_target(), RHIAccess::RTV)
		        .add_resource(emissive_target(), RHIAccess::RTV)
		        .add_resource(msra_target(), RHIAccess::RTV)
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this]() { geometry_pass(); });

		ViewMode mode = view_mode();

		if (mode == ViewMode::Lit)
		{
			graph.add_pass(RenderGraph::Pass::Graphics, "Lighting Pass")
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
			graph.add_pass(RenderGraph::Pass::Graphics, "Base Color Resolve")
			        .add_resource(base_color_target(), RHIAccess::CopySrc)
			        .add_resource(scene_color_target(), RHIAccess::CopyDst)
			        .add_func([this]() { copy_base_color_to_scene_color(); });
		}
		return *this;
	}

	void DeferredRenderer::geometry_pass()
	{
		rhi->bind_render_target4(base_color_target()->as_rtv(), normal_target()->as_rtv(), emissive_target()->as_rtv(),
		                         msra_target()->as_rtv(), scene_depth_target()->as_dsv());
		render_visible_primitives(RenderPasses::GenericGeometry::static_instance());
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

	void DeferredRenderer::deferred_lighting_pass()
	{
		RHI_Sampler* sampler = Sampler(SamplerFilter::Point).rhi_sampler();

		auto pipeline = Pipelines::AmbientLight::instance();
		rhi->bind_render_target1(scene_color_target()->as_rtv());

		pipeline->rhi_bind();

		rhi->bind_uniform_buffer(globals_uniform_buffer(), pipeline->globals->location);
		rhi->update_scalar_parameter(&scene()->environment.ambient_color, pipeline->ambient_color);
		rhi->bind_srv(base_color_target()->as_srv(), pipeline->base_color->location, sampler);
		rhi->bind_srv(msra_target()->as_srv(), pipeline->msra->location, sampler);

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

			rhi->bind_srv(base_color_target()->as_srv(), pipeline->base_color_texture->location, sampler);
			rhi->bind_srv(normal_target()->as_srv(), pipeline->normal_texture->location, sampler);
			rhi->bind_srv(emissive_target()->as_srv(), pipeline->emissive_texture->location, sampler);
			rhi->bind_srv(msra_target()->as_srv(), pipeline->msra_texture->location, sampler);
			rhi->bind_srv(scene_depth_target()->as_srv(), pipeline->depth_texture->location, sampler);
			rhi->bind_uniform_buffer(globals_uniform_buffer(), pipeline->globals->location);
			rhi->update_scalar_parameter(&parameters, pipeline->parameters);
			rhi->draw(6, 0);
		}
	}

	void DeferredRenderer::copy_base_color_to_scene_color()
	{
		auto src = base_color_target()->as_rtv();
		auto dst = scene_color_target()->as_rtv();

		Rect2D rect({}, scene_view().view_size());
		dst->blit(src, rect, rect, SamplerFilter::Point);
	}
}// namespace Engine
