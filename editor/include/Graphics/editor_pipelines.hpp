#pragma once
#include <Graphics/pipeline.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
	class Renderer;
	class RHIShaderResourceView;
	class RHIContext;
}// namespace Engine

namespace Engine::EditorPipelines
{
	class Outline : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(Outline, GlobalGraphicsPipeline);

		const RHIShaderParameterInfo* m_scene_color;
		const RHIShaderParameterInfo* m_scene_depth;
		const RHIShaderParameterInfo* m_outline_depth;
		const RHIShaderParameterInfo* m_sampler;
		const RHIShaderParameterInfo* m_outline_color;
		const RHIShaderParameterInfo* m_near;
		const RHIShaderParameterInfo* m_far;
		const RHIShaderParameterInfo* m_sample_offset;

	public:
		using SRV = RHIShaderResourceView;
		void render(RHIContext* ctx, Renderer* renderer, SRV* scene_color, SRV* outlines_depth, Vector3f color);
		void render(RHIContext* ctx, Renderer* renderer, SRV* scene_color, SRV* outlines_depth, Vector3f color,
		            Vector2f sample_offset);
	};

	class Grid : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(Grid, GlobalGraphicsPipeline);

	private:
		const RHIShaderParameterInfo* m_scene_view;
		const RHIShaderParameterInfo* m_fov;

	public:
		void render(RHIContext* ctx, Renderer* renderer);
	};

}// namespace Engine::EditorPipelines
