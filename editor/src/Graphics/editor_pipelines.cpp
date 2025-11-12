#include <Core/engine_loading_controllers.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/frustum.hpp>
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
		m_args       = find_parameter("args");
	}

	void Grid::render(RHIContext* ctx, Renderer* renderer)
	{
		constexpr float scale_step = 10.0;

		Args args;

		auto& projection = renderer->scene_view().camera_view().projection;
		auto forward     = Plane::static_near(projection).normal;
		auto bottom      = Plane::static_bottom(projection).normal;

		float angle             = (Math::half_pi() - Math::angle(forward, bottom));
		float perspective_scale = Math::abs(Math::tan(angle));

		if (perspective_scale == 0.f)
		{
			args.lower_scale = 1.f;
			args.upper_scale = 1.f;
			args.lower_alpha = 1.f;
			args.upper_alpha = 0.f;
		}
		else
		{
			args.lower_scale = 1.f;
			args.upper_scale = scale_step;

			float camera_height = Math::abs(renderer->scene_view().camera_view().location().y);
			float log_scale     = Math::log(camera_height / (perspective_scale)) / Math::log(scale_step);

			int32_t step = Math::max(0, (int32_t) Math::floor(log_scale));

			float lower_height = perspective_scale * Math::pow(scale_step, step);
			float upper_height = lower_height * scale_step;

			args.lower_scale = Math::pow(scale_step, step);
			args.upper_scale = args.lower_scale * scale_step;

			args.upper_alpha = (camera_height - lower_height) / (upper_height - lower_height);
			args.lower_alpha = 1.0f - args.upper_alpha;
		}

		ctx->depth_state(RHIDepthState(true, RHICompareFunc::Lequal, false));
		ctx->stencil_state(RHIStencilState());
		ctx->blending_state(RHIBlendingState(true));
		ctx->bind_pipeline(rhi_pipeline());

		ctx->push_primitive_topology(RHIPrimitiveTopology::TriangleList);
		ctx->push_cull_mode(RHICullMode::None);

		ctx->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		ctx->update_scalar(&args, m_args);
		ctx->draw(6, 0);

		ctx->pop_cull_mode();
		ctx->pop_primitive_topology();
	}
}// namespace Engine::EditorPipelines
