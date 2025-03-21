#pragma once
#include <Graphics/pipeline.hpp>

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;
	struct RHI_Sampler;

	namespace Pipelines
	{
		// clang-format off
		trinex_declare_compute_pipeline(GaussianBlur,
			private:
				const ShaderParameterInfo* m_src;
				const ShaderParameterInfo* m_dst;
				const ShaderParameterInfo* m_sigma;
				const ShaderParameterInfo* m_kernel_size;
			public:

			void blur(RHI_ShaderResourceView* src, RHI_UnorderedAccessView* dst, const Vector2u& dst_size, int32_t kernel = 5, float sigma = 2.f, RHI_Sampler* sampler = nullptr);
		);
		// clang-format on

	}// namespace Pipelines
}// namespace Engine
