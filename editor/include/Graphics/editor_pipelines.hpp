#pragma once
#include <Graphics/pipeline.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
	class Renderer;
	struct RHI_ShaderResourceView;
}// namespace Engine

namespace Engine::EditorPipelines
{
	class Outline : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(Outline, GlobalGraphicsPipeline);

		const ShaderParameterInfo* m_scene_color;
		const ShaderParameterInfo* m_scene_depth;
		const ShaderParameterInfo* m_outline_depth;
		const ShaderParameterInfo* m_linear;
		const ShaderParameterInfo* m_point;
		const ShaderParameterInfo* m_outline_color;
		const ShaderParameterInfo* m_near;
		const ShaderParameterInfo* m_far;
		const ShaderParameterInfo* m_sample_offset;

		Sampler m_point_sampler;
		Sampler m_linear_sampler;

	public:
		using SRV = RHI_ShaderResourceView;
		void render(Renderer* renderer, SRV* outline_depth, Vector3f color);
		void render(Renderer* renderer, SRV* outline_depth, Vector3f color, Vector2f sample_offset);
	};

	class Grid : public GlobalGraphicsPipeline
	{
		trinex_declare_pipeline(Grid, GlobalGraphicsPipeline);

	private:
		const ShaderParameterInfo* m_scene_view;

	public:
		void render(Renderer* renderer);
	};

}// namespace Engine::EditorPipelines
