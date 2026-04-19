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

		NoneApi& update(float dt) override;
		NoneApi& submit(const RHISubmitInfo& info) override;
		NoneApi& idle() override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHIFence* create_fence() override;
		RHISampler* create_sampler(const RHISamplerDesc& desc) override;
		RHITexture* create_texture(const RHITextureDesc& desc) override;
		RHIShader* create_shader(Span<u8> source, Span<RHIShaderParameterInfo> parameters) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineDesc& desc) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineDesc& desc) override;
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc) override;
		RHIBuffer* create_buffer(usize size, RHIBufferFlags type) override;
		RHISwapchain* create_swapchain(Window* window, bool vsync) override;
		RHIContext* create_context(RHIContextFlags flags) override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const u8* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size) override;

		NoneApi& present(RHISwapchain* swapchain) override;
	};
}// namespace Trinex
