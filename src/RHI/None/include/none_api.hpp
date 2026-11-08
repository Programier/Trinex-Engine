#pragma once

#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	class NoneApi : public RHI
	{
	public:
		const char* name() const override { return "None"; }
		const char* renderer() const override { return "None"; }

		void wait() override;
		RHIQueue* queue(RHIQueueType type, RHISwapchain* swapchain = nullptr) override;
		RHIArrayView<RHIQueue*> queues() const override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHIFence* create_fence(u64 value = 0) override;
		RHISampler* create_sampler(const RHISamplerDesc& desc) override;
		RHITexture* create_texture(const RHITextureDesc& desc) override;
		RHIShader* create_shader(Span<u8> source, Span<RHIBinding> parameters) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineDesc& desc) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineDesc& desc) override;
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc) override;
		RHIBuffer* create_buffer(usize size, RHIBufferFlags type) override;
		RHISwapchain* create_swapchain(Window* window, RHIPresentMode mode, u32 images = 0) override;
		RHIContext* create_context(RHIQueue* queue, RHIContextFlags flags) override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const u8* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size) override;
	};
} // namespace Trinex
