#include <Core/engine_loading_controllers.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/editor_pipelines.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>

namespace Engine::EditorPipelines
{
	trinex_implement_pipeline(Outline, "[shaders_dir]:/TrinexEditor/outlines.slang")
	{
		m_scene_color   = find_parameter("args.scene_color");
		m_scene_depth   = find_parameter("args.scene_depth");
		m_outline_depth = find_parameter("args.outline_depth");
		m_sampler       = find_parameter("args.sampler");
		m_outline_color = find_parameter("args.outline_color");
		m_near          = find_parameter("args.near");
		m_far           = find_parameter("args.far");
		m_sample_offset = find_parameter("args.sample_offset");
	}

	void Outline::render(RHIContext* ctx, Renderer* renderer, SRV* scene_color, SRV* outline_depth, Vector3f color)
	{
		auto view_size = Vector2f(renderer->scene_view().view_size());
		float offset   = glm::max(1.f, glm::max(view_size.x, view_size.y) / 512.f);

		auto sample_offset = Vector2f(offset, offset) / view_size;
		render(ctx, renderer, scene_color, outline_depth, color, sample_offset);
	}

	void Outline::render(RHIContext* ctx, Renderer* renderer, SRV* scene_color, SRV* outline_depth, Vector3f color,
	                     Vector2f sample_offset)
	{
		ctx->bind_render_target1(renderer->scene_color_ldr_target()->as_rtv());

		SRV* scene_depth = renderer->scene_depth_target()->as_srv();

		ctx->bind_pipeline(rhi_pipeline());
		ctx->bind_srv(scene_color, m_scene_color->binding);
		ctx->bind_srv(scene_depth, m_scene_depth->binding);
		ctx->bind_srv(outline_depth, m_outline_depth->binding);

		ctx->bind_sampler(RHIPointSampler::static_sampler(), m_sampler->binding);
		ctx->update_scalar(&color, m_outline_color);

		auto& camera_view = renderer->scene_view().camera_view();
		ctx->update_scalar(&camera_view.near, m_near);
		ctx->update_scalar(&camera_view.far, m_far);
		ctx->update_scalar(&sample_offset, m_sample_offset);
		ctx->draw(6, 0);
	}

	trinex_implement_pipeline(Grid, "[shaders_dir]:/TrinexEditor/grid.slang")
	{
		m_scene_view = find_parameter("scene_view");
		m_fov        = find_parameter("fov");

		depth_test.enable       = true;
		depth_test.write_enable = false;
		depth_test.func         = RHICompareFunc::Lequal;
		color_blending.enable   = true;
	}

	void Grid::render(RHIContext* ctx, Renderer* renderer)
	{
		float fov = Math::tan(Math::radians(renderer->scene_view().camera_view().perspective.fov));

		ctx->bind_pipeline(rhi_pipeline());
		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->update_scalar(&fov, m_fov);
		ctx->draw(6, 0);
	}
}// namespace Engine::EditorPipelines
