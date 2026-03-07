#pragma once

#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>


namespace Trinex
{
	class NoneApi : public RHI
	{
	public:
		trinex_struct(NoneApi, void);
		static NoneApi* static_constructor();
		static void static_destructor(NoneApi* api);

		static NoneApi* m_instance;

		NoneApi& signal(RHIFence* fence) override;
		NoneApi& submit(RHICommandHandle*) override;
		NoneApi& idle() override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHIFence* create_fence() override;
		RHISampler* create_sampler(const RHISamplerInitializer*) override;
		RHITexture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, u32 mips,
		                           RHITextureCreateFlags flags) override;
		RHIShader* create_shader(const u8* source, usize size) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline) override;
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineInitializer* pipeline) override;
		RHIBuffer* create_buffer(usize size, RHIBufferCreateFlags type) override;
		RHISwapchain* create_swapchain(Window* window, bool vsync) override;
		RHIContext* create_context(RHIContextFlags flags) override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const u8* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size) override;

		NoneApi& present(RHISwapchain* swapchain) override;
	};
}// namespace Trinex
