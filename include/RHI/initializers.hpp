#pragma once
#include <Core/types/color.hpp>
#include <RHI/pipeline.hpp>

namespace Engine
{
	class RHIShader;

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
				RHIShader* vertex_shader;
				RHIShader* tessellation_control_shader;
				RHIShader* tessellation_shader;
				RHIShader* geometry_shader;
				RHIShader* fragment_shader;
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

		inline RHIGraphicsPipelineInitializer()
		    : shaders{nullptr}, parameters(nullptr), vertex_attributes(nullptr), parameters_count(0), vertex_attributes_count(0),
		      depth(), stencil(), blending()
		{}
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
		RHIShader* compute_shader                       = nullptr;
		const struct RHIShaderParameterInfo* parameters = nullptr;
		size_t parameters_count                         = 0;
	};

	struct ENGINE_EXPORT RHIRayTracingShaderGroup {
		union
		{
			RHIShader* general = nullptr;
			RHIShader* intersection;
		};

		RHIShader* closest_hit = nullptr;
		RHIShader* any_hit     = nullptr;

		// General* — RayGen, Miss, Callable
		// TrianglesHit - ClosestHit + AnyHit
		// ProceduralHit - Intersection + ClosestHit + AnyHit
		RHIRayTracingShaderGroupType type = RHIRayTracingShaderGroupType::GeneralRayGen;
	};

	struct ENGINE_EXPORT RHIRayTracingPipelineInitializer {
		RHIRayTracingShaderGroup* groups = nullptr;
		uint64_t groups_count            = 0;

		const struct RHIShaderParameterInfo* parameters = nullptr;
		size_t parameters_count                         = 0;

		byte max_recursion = 2;
	};
}// namespace Engine
