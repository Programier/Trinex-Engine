#include "Engine/Actors/actor.hpp"
#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/editor_pipelines.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>

namespace Engine::EditorRenderer
{
	void render_grid(Renderer* renderer)
	{
		if (!Settings::Editor::show_grid)
			return;

		renderer->add_custom_pass([](Renderer* renderer, RenderGraph::Graph& graph) {
			graph.add_pass(RenderGraph::Pass::Graphics, "Editor Grid")
			        .add_resource(renderer->scene_color_target(), RHIAccess::RTV)
			        .add_resource(renderer->scene_depth_target(), RHIAccess::DSV)
			        .add_func([renderer]() {
				        rhi->bind_render_target1(renderer->scene_color_target()->as_rtv(),
				                                 renderer->scene_depth_target()->as_dsv());
				        RendererContext ctx(renderer, RenderPasses::GenericOutput::static_instance());
				        EditorResources::grid_material->apply(ctx);
				        rhi->draw(6, 0);
			        });
		});
	}

	static void render_outlines_pass(Renderer* renderer, Actor** actors, size_t count)
	{
		auto view_size = renderer->scene_view().view_size();
		auto pool      = RHISurfacePool::global_instance();
		auto depth     = pool->request_surface(SurfaceFormat::Depth, view_size);

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
		renderer->add_custom_pass([actors, count](Renderer* renderer, RenderGraph::Graph& graph) {
			graph.add_pass(RenderGraph::Pass::Graphics, "Editor Outlines")
			        .add_resource(renderer->scene_color_target(), RHIAccess::RTV)
			        .add_resource(renderer->scene_depth_target(), RHIAccess::SRVGraphics)
			        .add_func([renderer, actors, count]() { render_outlines_pass(renderer, actors, count); });
		});
	}

	void render_primitives(Renderer* renderer, Actor** actors, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
		{
			Actor* actor = actors[i];

			for (ActorComponent* component : actor->owned_components())
			{
				if (auto spot_light = Object::instance_cast<SpotLightComponent>(component))
				{
					auto proxy = spot_light->proxy();

					auto dir           = proxy->direction() * 2.f;
					float outer_radius = 2.f * glm::tan(proxy->outer_cone_angle());
					float inner_radius = 2.f * glm::tan(proxy->inner_cone_angle());

					auto location = proxy->world_transform().location() + dir;
					renderer->lines.add_cone(location, -dir, outer_radius, {255, 255, 0, 255}, 0, 3.f);
					renderer->lines.add_cone(location, -dir, inner_radius, {255, 255, 0, 255}, 0, 3.f);
				}
				else if (auto point_light = Object::instance_cast<PointLightComponent>(component))
				{
					auto proxy     = point_light->proxy();
					auto& location = proxy->world_transform().location();
					renderer->lines.add_sphere(location, proxy->attenuation_radius(), {255, 255, 0, 255}, 0, 3.f);
				}
				else if (auto directional_light = Object::instance_cast<DirectionalLightComponent>(component))
				{
					auto proxy     = directional_light->proxy();
					auto& location = proxy->world_transform().location();
					renderer->lines.add_arrow(location, proxy->direction(), {255, 255, 0, 255}, 3.f);
				}
			}
		}
	}
}// namespace Engine::EditorRenderer
