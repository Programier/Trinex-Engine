#pragma once
#include <Core/etl/span.hpp>
#include <Core/etl/string.hpp>
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
	class RHIGarbageCollector;

	struct RHITextureDesc;
	struct RHISamplerDesc;
	struct RHIGraphicsPipelineDesc;
	struct RHIMeshPipelineDesc;
	struct RHIComputePipelineDesc;
	struct RHIRayTracingPipelineDesc;

	class trinex_api RHI
	{
	public:
		using CreateFunction  = RHI* (*) ();
		using DestroyFunction = void (*)(RHI*);

		class trinex_api Registry
		{
		public:
			Registry(RHIBackend backend, CreateFunction create, DestroyFunction destroy);
		};

	private:
		RHIGarbageCollector* m_gc = nullptr;

	public:
		static constexpr usize s_max_srv             = 32;
		static constexpr usize s_max_samplers        = 32;
		static constexpr usize s_max_uav             = 8;
		static constexpr usize s_max_uniform_buffers = 8;
		static constexpr usize s_max_vertex_buffers  = 8;

		static bool is_supported(RHIBackend backend);
		static RHIBackend backend();
		static RHI* instance();
		static RHI* instance(RHIBackend backend);
		inline RHIGarbageCollector* gc() const { return m_gc; }

	protected:
		RHI();
		virtual ~RHI();

	public:
		virtual const char* name() const     = 0;
		virtual const char* renderer() const = 0;

		virtual void wait()                                                           = 0;
		virtual RHIQueue* queue(RHIQueueType type, RHISwapchain* swapchain = nullptr) = 0;
		virtual RHIArrayView<RHIQueue*> queues() const                                = 0;

		virtual RHITimestamp* create_timestamp()                                                         = 0;
		virtual RHIPipelineStatistics* create_pipeline_statistics()                                      = 0;
		virtual RHIFence* create_fence(u64 value = 0)                                                    = 0;
		virtual RHISampler* create_sampler(const RHISamplerDesc& desc)                                   = 0;
		virtual RHITexture* create_texture(const RHITextureDesc& desc)                                   = 0;
		virtual RHIShader* create_shader(const void* data, usize size)                                   = 0;
		virtual RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc)               = 0;
		virtual RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineDesc& desc)                       = 0;
		virtual RHIPipeline* create_compute_pipeline(const RHIComputePipelineDesc& desc)                 = 0;
		virtual RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc)          = 0;
		virtual RHIBuffer* create_buffer(usize size, RHIBufferFlags flags)                               = 0;
		virtual RHISwapchain* create_swapchain(Window* window, RHIPresentMode mode, u32 images = 0)      = 0;
		virtual RHIContext* create_context(RHIQueue* queue)                                              = 0;
		virtual RHIBindingLayout* create_binding_layout(RHIBinding* bindings = nullptr, usize count = 0) = 0;
		virtual RHIBindingSet* create_binding_set(RHIBindingLayout* layout)                              = 0;

		// Raytracing
		virtual RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) = 0;
		virtual const u8* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size) = 0;
	};

	template<typename T>
	class RHIRegistry : public RHI::Registry
	{
	private:
		static RHI* create_instance() { return trx_new T(); }
		static void destroy_instance(RHI* api) { trx_delete static_cast<T*>(api); }

	public:
		RHIRegistry(RHIBackend backend) : RHI::Registry(backend, create_instance, destroy_instance) {}
	};
} // namespace Trinex
