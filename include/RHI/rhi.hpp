#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>
#include <RHI/handles.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class Window;
	class GraphicsPipeline;
	class ComputePipeline;

	class RHIContext;
	class RHICommandHandle;

	struct RHISamplerInitializer;
	struct RHIGraphicsPipelineInitializer;
	struct RHIMeshPipelineInitializer;
	struct RHIComputePipelineInitializer;
	struct RHIRayTracingPipelineInitializer;

	namespace Refl
	{
		class Struct;
	}

	class ENGINE_EXPORT RHI
	{
	public:
		static constexpr size_t s_max_srv             = 32;
		static constexpr size_t s_max_samplers        = 32;
		static constexpr size_t s_max_uav             = 8;
		static constexpr size_t s_max_uniform_buffers = 8;
		static constexpr size_t s_max_vertex_buffers  = 8;

		struct Info {
			String name;
			String renderer;
			Refl::Struct* struct_instance = nullptr;
		} info;

		virtual RHI& signal(RHIFence* fence)          = 0;
		virtual RHI& submit(RHICommandHandle* handle) = 0;
		virtual RHI& idle()                           = 0;

		virtual RHITimestamp* create_timestamp()                                                           = 0;
		virtual RHIPipelineStatistics* create_pipeline_statistics()                                        = 0;
		virtual RHIFence* create_fence()                                                                   = 0;
		virtual RHISampler* create_sampler(const RHISamplerInitializer*)                                   = 0;
		virtual RHITexture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                                   RHITextureCreateFlags flags)                                    = 0;
		virtual RHIShader* create_shader(const byte* shader, size_t size)                                  = 0;
		virtual RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)      = 0;
		virtual RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline)              = 0;
		virtual RHIPipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)        = 0;
		virtual RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineInitializer* pipeline) = 0;
		virtual RHIBuffer* create_buffer(size_t size, RHIBufferCreateFlags flags)                          = 0;
		virtual RHISwapchain* create_swapchain(Window* window, bool vsync)                                 = 0;
		virtual RHIContext* create_context()                                                               = 0;

		// Raytracing
		virtual RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs)    = 0;
		virtual const byte* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, size_t& size) = 0;

		virtual RHI& present(RHISwapchain* swapchain) = 0;

		virtual ~RHI() {};
	};

	ENGINE_EXPORT extern RHI* rhi;
}// namespace Engine
