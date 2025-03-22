#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>

namespace Engine::Pipelines
{
	trinex_implement_pipeline(GaussianBlur, "[shaders_dir]:/TrinexEngine/trinex/compute/gaussian_blur.slang", ShaderType::Compute)
	{
		m_src         = find_param_info("input");
		m_dst         = find_param_info("output");
		m_sigma       = find_param_info("sigma");
		m_kernel_size = find_param_info("kernel_size");
	}

	void GaussianBlur::blur(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Vector2u& dst_size, int32_t kernel,
							float sigma, RHI_Sampler* sampler)
	{
		kernel = glm::abs(kernel);
		sigma  = glm::abs(sigma);

		if (sampler == nullptr)
			sampler = DefaultResources::Samplers::default_sampler->rhi_sampler();

		rhi_bind();

		src->bind_combined(m_src->location, sampler);
		dst->bind(m_dst->location);

		rhi->update_scalar_parameter(&kernel, sizeof(kernel), m_kernel_size->offset, m_kernel_size->location);
		rhi->update_scalar_parameter(&sigma, sizeof(sigma), m_sigma->offset, m_sigma->location);

		// Shader uses 8x8x1 threads per group
		Vector2u groups = {(dst_size.x + 7) / 8, (dst_size.y + 7) / 8};
		rhi->dispatch(groups.x, groups.y, 1);
	}

	trinex_implement_pipeline(BatchedLines, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_lines.slang",
							  ShaderType::Vertex | ShaderType::Geometry | ShaderType::Fragment)
	{
		input_assembly.primitive_topology = PrimitiveTopology::LineList;
		color_blending.enable             = true;

		m_globals = find_param_info("globals");
	}

	void BatchedLines::apply(SceneRenderer* renderer)
	{
		rhi_bind();
		renderer->bind_global_parameters(m_globals->location);
	}

	trinex_implement_pipeline(BatchedTriangles, "[shaders_dir]:/TrinexEngine/trinex/graphics/batched_triangles.slang",
							  ShaderType::Vertex | ShaderType::Fragment)
	{
		input_assembly.primitive_topology = PrimitiveTopology::TriangleList;
		color_blending.enable             = true;
	}
}// namespace Engine::Pipelines
