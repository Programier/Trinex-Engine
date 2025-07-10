#include <Engine/Render/depth_renderer.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	DepthRenderer::DepthRenderer(Scene* scene, const SceneView& view, ViewMode mode) : Renderer(scene, view, mode)
	{
		auto graph = render_graph();

		graph->add_output(scene_depth_target());
		graph->add_pass(RenderGraph::Pass::Graphics, "Depth Rendering")
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this]() { render_depth(); });
	}

	DepthRenderer& DepthRenderer::render_depth()
	{
		Frustum frustum = scene_view().camera_view();

		FrameVector<PrimitiveComponent*> components = scene()->collect_visible_primitives(frustum);
		rhi->bind_depth_stencil_target(scene_depth_target()->as_dsv());

		for (PrimitiveComponent* component : components)
		{
			render_primitive(RenderPasses::Depth::static_instance(), component);
		}
		return *this;
	}
}// namespace Engine
