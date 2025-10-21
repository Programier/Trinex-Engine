#pragma once

#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>


namespace Engine
{
	class NoneApi : public RHI
	{
	public:
		trinex_declare_struct(NoneApi, void);
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
		RHITexture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                           RHITextureCreateFlags flags) override;
		RHIShader* create_shader(const byte* source, size_t size) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline) override;
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineInitializer* pipeline) override;
		RHIBuffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags type) override;
		RHISwapchain* create_swapchain(Window* window, bool vsync) override;
		RHIContext* create_context() override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const byte* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, size_t& size) override;

		NoneApi& present(RHISwapchain* swapchain) override;
	};
}// namespace Engine
