#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	static void render_editor_grid(Renderer* renderer, RenderGraph::Graph& graph)
	{

		graph.add_pass(RenderGraph::Pass::Graphics, "Editor Grid")
		        .add_resource(renderer->scene_color_target(), RHIAccess::RTV)
		        .add_resource(renderer->scene_depth_target(), RHIAccess::DSV)
		        .add_func([renderer]() {
			        rhi->bind_render_target1(renderer->scene_color_target()->as_rtv(), renderer->scene_depth_target()->as_dsv());
			        RendererContext ctx(renderer, RenderPasses::GenericOutput::static_instance());
			        EditorResources::grid_material->apply(ctx);
			        rhi->draw(6, 0);
		        });
	}

	void register_editor_render_passes(Renderer* renderer)
	{
		if (Settings::Editor::show_grid)
			renderer->add_custom_pass(render_editor_grid);
	}
}// namespace Engine
