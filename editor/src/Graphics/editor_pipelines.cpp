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

	void Outline::render(Renderer* renderer, SRV* outline_depth, Vector3f color)
	{
		auto view_size = Vector2f(renderer->scene_view().view_size());
		float offset   = glm::max(1.f, glm::max(view_size.x, view_size.y) / 512.f);

		auto sample_offset = Vector2f(offset, offset) / view_size;
		render(renderer, outline_depth, color, sample_offset);
	}

	void Outline::render(Renderer* renderer, SRV* outline_depth, Vector3f color, Vector2f sample_offset)
	{
		auto view_size  = renderer->scene_view().view_size();
		auto tmp_format = renderer->static_surface_format_of(Renderer::SceneColorLDR);
		auto tmp_color  = RHITexturePool::global_instance()->request_surface(tmp_format, view_size);

		RHITextureRegion region = {view_size};
		rhi->context()->copy_texture_to_texture(renderer->scene_color_ldr_target(), region, tmp_color, region);
		rhi->context()->bind_render_target1(renderer->scene_color_ldr_target()->as_rtv());

		SRV* scene_color = tmp_color->as_srv();
		SRV* scene_depth = renderer->scene_depth_target()->as_srv();

		rhi_bind();
		rhi->context()->bind_srv(scene_color, m_scene_color->binding);
		rhi->context()->bind_srv(scene_depth, m_scene_depth->binding);
		rhi->context()->bind_srv(outline_depth, m_outline_depth->binding);

		rhi->context()->bind_sampler(RHIPointSampler::static_sampler(), m_sampler->binding);
		rhi->context()->update_scalar(&color, m_outline_color);

		auto& camera_view = renderer->scene_view().camera_view();
		rhi->context()->update_scalar(&camera_view.near, m_near);
		rhi->context()->update_scalar(&camera_view.far, m_far);
		rhi->context()->update_scalar(&sample_offset, m_sample_offset);
		rhi->context()->draw(6, 0);

		RHITexturePool::global_instance()->return_surface(tmp_color);
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

	void Grid::render(Renderer* renderer)
	{
		float fov = Math::tan(Math::radians(renderer->scene_view().camera_view().perspective.fov));

		rhi_bind();
		rhi->context()->bind_uniform_buffer(renderer->globals_uniform_buffer(), m_scene_view->binding);
		rhi->context()->update_scalar(&fov, m_fov);
		rhi->context()->draw(6, 0);
	}
}// namespace Engine::EditorPipelines
