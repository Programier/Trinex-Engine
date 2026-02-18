#include <Core/math/math.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/depth_renderer.hpp>
#include <Engine/Render/primitive_context.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/camera_view.hpp>
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
		graph->add_pass("Depth Rendering").add_resource(scene_depth_target(), RHIAccess::DSV).add_func([this](RHIContext* cxt) {
			render_depth(cxt);
		});
	}

	DepthRenderer& DepthRenderer::render_depth(RHIContext* ctx)
	{
		Frustum frustum = scene_view().camera_view().projview;

		FrameVector<PrimitiveComponent*> components = scene()->collect_visible_primitives(frustum);
		ctx->begin_rendering(scene_depth_target()->as_dsv());
		{
			for (PrimitiveComponent* component : components)
			{
				Matrix4f local_to_world = component->world_transform().matrix();
				PrimitiveRenderingContext context(this, ctx, RenderPasses::Depth::static_instance(), &local_to_world);
				component->render(&context);
			}
		}
		ctx->end_rendering();
		return *this;
	}

	DepthCubeRenderer::DepthCubeRenderer(Scene* scene, const SceneView& view, ViewMode mode) : Renderer(scene, view, mode)
	{
		auto pool = RHITexturePool::global_instance();
		m_cubemap = pool->request_transient_surface(RHITextureType::TextureCube, RHISurfaceFormat::D32F, {view.view_size(), 1});

		auto graph = render_graph();

		graph->add_output(cubemap());

		graph->add_pass("Depth Rendering")
		        .add_resource(cubemap(), RHIAccess::DSV, RHIAccess::TransferDst)
		        .add_func([this](RHIContext* ctx) { render_depth(ctx); });
	}

	DepthCubeRenderer& DepthCubeRenderer::render(RHIContext* cxt)
	{
		Renderer::render(cxt);
		return *this;
	}

	DepthCubeRenderer& DepthCubeRenderer::clear_depth(RHIContext* ctx)
	{
		trinex_rhi_push_stage(ctx, "Clear");
		ctx->clear_dsv(cubemap()->as_dsv());
		ctx->barrier(cubemap(), RHIAccess::DSV);
		trinex_rhi_pop_stage(ctx);
		return *this;
	}

	DepthCubeRenderer& DepthCubeRenderer::render_depth(RHIContext* ctx, const Vector3f& forward, const Vector3f& up, uint_t face)
	{
		const auto& current_scene_view = scene_view();

		CameraView camera = scene_view().camera_view();
		camera.look(camera.location(), forward, up);

		reset(SceneView(camera, current_scene_view.view_size(), current_scene_view.show_flags()));

#if TRINEX_DEBUG_BUILD
		static const char* face_names[6] = {"Right ", "Left", "Top", "Bottom", "Front", "Back"};
		ctx->push_debug_stage(face_names[face]);
#endif

		Frustum frustum                             = scene_view().camera_view().projview;
		FrameVector<PrimitiveComponent*> components = scene()->collect_visible_primitives(frustum);

		RHITextureDescDSV view;
		view.base_slice  = face;
		view.slice_count = 1;
		view.view_type   = RHITextureType::Texture2D;

		auto dsv = cubemap()->as_dsv(&view);

		ctx->begin_rendering(dsv);
		{
			for (PrimitiveComponent* component : components)
			{
				Matrix4f local_to_world = component->world_transform().matrix();
				PrimitiveRenderingContext context(this, ctx, RenderPasses::Depth::static_instance(), &local_to_world);
				component->render(&context);
			}
		}
		ctx->end_rendering();

#if TRINEX_DEBUG_BUILD
		ctx->pop_debug_stage();
#endif

		return *this;
	}

	DepthCubeRenderer& DepthCubeRenderer::render_depth(RHIContext* ctx)
	{
		clear_depth(ctx);

		// Back (-Z)
		render_depth(ctx, {0.f, 0.f, -1.f}, {0.f, -1.f, 0.f}, RHICubeFace::Back);

		// Front (+Z)
		render_depth(ctx, {0.f, 0.f, 1.f}, {0.f, -1.f, 0.f}, RHICubeFace::Front);

		// Right (+X)
		render_depth(ctx, {1.f, 0.f, 0.f}, {0.f, -1.f, 0.f}, RHICubeFace::Right);

		// Left (-X)
		render_depth(ctx, {-1.f, 0.f, 0.f}, {0.f, -1.f, 0.f}, RHICubeFace::Left);

		// Top (+Y)
		render_depth(ctx, {0.f, 1.f, 0.f}, {0.f, 0.f, -1.f}, RHICubeFace::Top);

		// Bottom (-Y)
		render_depth(ctx, {0.f, -1.f, 0.f}, {0.f, 0.f, 1.f}, RHICubeFace::Bottom);

		return *this;
	}
}// namespace Engine
