#include <Core/math/math.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/depth_renderer.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
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
		Frustum frustum = scene_view().projview();

		FrameVector<PrimitiveComponent*> components = scene()->collect_visible_primitives(frustum);
		rhi->context()->bind_depth_stencil_target(scene_depth_target()->as_dsv());

		for (PrimitiveComponent* component : components)
		{
			component->proxy()->render(this, RenderPasses::Depth::static_instance());
		}
		return *this;
	}

	DepthCubeRenderer::DepthCubeRenderer(Scene* scene, const SceneView& view, ViewMode mode) : Renderer(scene, view, mode)
	{
		auto pool = RHITexturePool::global_instance();
		m_cubemap = pool->request_transient_surface(RHITextureType::TextureCube, RHISurfaceFormat::D32F, {view.view_size(), 1});

		auto graph = render_graph();

		graph->add_output(scene_depth_target());
		graph->add_pass(RenderGraph::Pass::Graphics, "Depth Rendering")
		        .add_resource(scene_depth_target(), RHIAccess::DSV)
		        .add_func([this]() { render_depth(); });
	}

	DepthCubeRenderer& DepthCubeRenderer::render_depth(CameraView& camera, uint_t face)
	{
		const auto& current_scene_view = scene_view();
		reset(SceneView(camera, current_scene_view.view_size(), current_scene_view.show_flags()));

#if TRINEX_DEBUG_BUILD
		static const char* face_names[6] = {"Right ", "Left", "Top", "Bottom", "Front", "Back"};
		rhi->context()->push_debug_stage(face_names[face]);
#endif

		Frustum frustum                             = scene_view().projview();
		FrameVector<PrimitiveComponent*> components = scene()->collect_visible_primitives(frustum);

		RHITextureDescDSV view;
		view.base_slice  = face;
		view.slice_count = 1;
		view.view_type   = RHITextureType::Texture2D;

		auto dsv = cubemap()->as_dsv(&view);

		rhi->context()->clear_dsv(dsv).bind_depth_stencil_target(dsv);

		for (PrimitiveComponent* component : components)
		{
			component->proxy()->render(this, RenderPasses::Depth::static_instance());
		}

#if TRINEX_DEBUG_BUILD
		rhi->context()->pop_debug_stage();
#endif

		return *this;
	}

	DepthCubeRenderer& DepthCubeRenderer::render_depth()
	{
		CameraView camera = scene_view().camera_view();

		// Back (-Z)
		camera.forward = {0.f, 0.f, -1.f};
		camera.up      = {0.f, -1.f, 0.f};
		camera.right   = {1.f, 0.f, 0.f};
		render_depth(camera, RHICubeFace::Back);

		// Front (+Z)
		camera.forward = {0.f, 0.f, 1.f};
		camera.up      = {0.f, -1.f, 0.f};
		camera.right   = {-1.f, 0.f, 0.f};
		render_depth(camera, RHICubeFace::Front);

		// Right (+X)
		camera.forward = {1.f, 0.f, 0.f};
		camera.up      = {0.f, -1.f, 0.f};
		camera.right   = {0.f, 0.f, 1.f};
		render_depth(camera, RHICubeFace::Right);

		// Left (-X)
		camera.forward = {-1.f, 0.f, 0.f};
		camera.up      = {0.f, -1.f, 0.f};
		camera.right   = {0.f, 0.f, -1.f};
		render_depth(camera, RHICubeFace::Left);

		// Top (+Y)
		camera.forward = {0.f, 1.f, 0.f};
		camera.up      = {0.f, 0.f, -1.f};
		camera.right   = {1.f, 0.f, 0.f};
		render_depth(camera, RHICubeFace::Top);

		// Bottom (-Y)
		camera.forward = {0.f, -1.f, 0.f};
		camera.up      = {0.f, 0.f, 1.f};
		camera.right   = {1.f, 0.f, 0.f};
		render_depth(camera, RHICubeFace::Bottom);

		return *this;
	}
}// namespace Engine
