#pragma once
#include <Core/types/color.hpp>
#include <RHI/pipeline.hpp>

namespace Engine
{
	struct RHIShader;

	struct ENGINE_EXPORT RHISamplerInitializer {
		RHISamplerFilter filter;
		RHISamplerAddressMode address_u;
		RHISamplerAddressMode address_v;
		RHISamplerAddressMode address_w;
		RHICompareFunc compare_func;
		Color border_color;

		float anisotropy;
		float mip_lod_bias;
		float min_lod;
		float max_lod;

		RHISamplerInitializer();
		HashIndex hash() const;

		bool operator==(const RHISamplerInitializer& initializer) const;
		inline bool operator!=(const RHISamplerInitializer& initializer) const { return !(*this == initializer); }
	};

	struct RHIGraphicsPipelineInitializer {
		union
		{
			struct {
				RHIShader* vertex_shader               = nullptr;
				RHIShader* tessellation_control_shader = nullptr;
				RHIShader* tessellation_shader         = nullptr;
				RHIShader* geometry_shader             = nullptr;
				RHIShader* fragment_shader             = nullptr;
			};

			RHIShader* shaders[5];
		};

		const struct RHIShaderParameterInfo* parameters    = nullptr;
		const struct RHIVertexAttribute* vertex_attributes = nullptr;
		size_t parameters_count                            = 0;
		size_t vertex_attributes_count                     = 0;

		RHIDepthTest depth;
		RHIStencilTest stencil;
		RHIColorBlending blending;
	};

	struct RHIMeshPipelineInitializer {
		union
		{
			struct {
				RHIShader* task_shader     = nullptr;
				RHIShader* mesh_shader     = nullptr;
				RHIShader* fragment_shader = nullptr;
			};

			RHIShader* shaders[3];
		};

		const struct RHIShaderParameterInfo* parameters = nullptr;
		size_t parameters_count                         = 0;

		RHIDepthTest depth;
		RHIStencilTest stencil;
		RHIColorBlending blending;
	};

	struct ENGINE_EXPORT RHIComputePipelineInitializer {
		RHIShader* compute_shader                      = nullptr;
		const struct RHIShaderParameterInfo* parameters = nullptr;
		size_t parameters_count                         = 0;
	};
}// namespace Engine
