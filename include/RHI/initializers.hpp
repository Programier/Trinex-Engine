#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	class RHIShader;

	struct RHITextureDesc {
		RHITextureType type         = RHITextureType::Texture2D;
		RHIColorFormat format       = RHIColorFormat::R8G8B8A8;
		Vector3u size               = {0, 0, 0};
		u32 mips                    = 1;
		RHITextureCreateFlags flags = RHITextureCreateFlags::ShaderResource;
	};

	struct ENGINE_EXPORT RHISamplerDesc {
		RHISamplerFilter filter;
		RHISamplerAddressMode address_u;
		RHISamplerAddressMode address_v;
		RHISamplerAddressMode address_w;
		RHICompareFunc compare_func;
		RHIBorderColor border_color;

		float anisotropy;
		float mip_lod_bias;
		float min_lod;
		float max_lod;

		RHISamplerDesc();
		u64 hash() const;

		bool operator==(const RHISamplerDesc& initializer) const;
		inline bool operator!=(const RHISamplerDesc& initializer) const { return !(*this == initializer); }
	};

	struct RHIGraphicsPipelineDesc {
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
		usize parameters_count                             = 0;
		usize vertex_attributes_count                      = 0;
	};

	struct RHIMeshPipelineDesc {
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
		usize parameters_count                          = 0;
	};

	struct ENGINE_EXPORT RHIComputePipelineDesc {
		RHIShader* compute_shader                       = nullptr;
		const struct RHIShaderParameterInfo* parameters = nullptr;
		usize parameters_count                          = 0;
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

	struct ENGINE_EXPORT RHIRayTracingPipelineDesc {
		RHIRayTracingShaderGroup* groups = nullptr;
		u64 groups_count                 = 0;

		const struct RHIShaderParameterInfo* parameters = nullptr;
		usize parameters_count                          = 0;

		u8 max_recursion = 2;
	};
}// namespace Trinex
