#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>
#include <RHI/handles.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class Window;
	class GraphicsPipeline;
	class ComputePipeline;

	class RHIContext;
	class RHICommandHandle;

	struct RHITextureDesc;
	struct RHISamplerDesc;
	struct RHIGraphicsPipelineDesc;
	struct RHIMeshPipelineDesc;
	struct RHIComputePipelineDesc;
	struct RHIRayTracingPipelineDesc;

	namespace Refl
	{
		class Struct;
	}

	class ENGINE_EXPORT RHI
	{
	private:
		static RHI* s_rhi;

	public:
		static constexpr usize s_max_srv             = 32;
		static constexpr usize s_max_samplers        = 32;
		static constexpr usize s_max_uav             = 8;
		static constexpr usize s_max_uniform_buffers = 8;
		static constexpr usize s_max_vertex_buffers  = 8;

		struct Info {
			String name;
			String renderer;
			Refl::Struct* struct_instance = nullptr;
		} info;

		static RHI* create(const char* name);
		static void destroy();
		static inline RHI* instance() { return s_rhi; }


		RHI();
		virtual ~RHI();

		virtual RHI& update(float dt)                  = 0;
		virtual RHI& submit(const RHISubmitInfo& info) = 0;
		virtual RHI& idle()                            = 0;

		virtual RHITimestamp* create_timestamp()                                                = 0;
		virtual RHIPipelineStatistics* create_pipeline_statistics()                             = 0;
		virtual RHIFence* create_fence()                                                        = 0;
		virtual RHISampler* create_sampler(const RHISamplerDesc& desc)                          = 0;
		virtual RHITexture* create_texture(const RHITextureDesc& desc)                          = 0;
		virtual RHIShader* create_shader(const u8* shader, usize size)                          = 0;
		virtual RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc)      = 0;
		virtual RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineDesc& desc)              = 0;
		virtual RHIPipeline* create_compute_pipeline(const RHIComputePipelineDesc& desc)        = 0;
		virtual RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc) = 0;
		virtual RHIBuffer* create_buffer(usize size, RHIBufferCreateFlags flags)                = 0;
		virtual RHISwapchain* create_swapchain(Window* window, bool vsync)                      = 0;
		virtual RHIContext* create_context(RHIContextFlags flags = RHIContextFlags::Undefined)  = 0;

		// Raytracing
		virtual RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) = 0;
		virtual const u8* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size) = 0;

		virtual RHI& present(RHISwapchain* swapchain) = 0;
	};
}// namespace Trinex
