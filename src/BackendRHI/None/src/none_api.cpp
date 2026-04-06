#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/struct.hpp>
#include <none_api.hpp>

namespace Trinex
{
	template<typename T>
	T* rhi_default()
	{
		static T t;
		return &t;
	}

	template<typename T>
	class NoneApiDestroyable : public T
	{
	public:
		using T::T;
		void destroy() override { delete this; }
	};

	NoneApi* NoneApi::m_instance = nullptr;

	NoneApi* NoneApi::static_constructor()
	{
		if (NoneApi::m_instance == nullptr)
		{
			NoneApi::m_instance                       = new NoneApi();
			NoneApi::m_instance->info.name            = "None";
			NoneApi::m_instance->info.renderer        = "None";
			NoneApi::m_instance->info.struct_instance = static_reflection();
		}
		return NoneApi::m_instance;
	}

	void NoneApi::static_destructor(NoneApi* api)
	{
		if (api == m_instance)
		{
			delete api;
			m_instance = nullptr;
		}
	}

	namespace TRINEX_RHI
	{
		using NONE = NoneApi;
	}

	trinex_implement_struct_default_init(Trinex::TRINEX_RHI::NONE, 0);

	struct NoneTimestamp : public NoneApiDestroyable<RHITimestamp> {
		float milliseconds() override { return 0.f; }
	};

	struct NonePipelineStatistics : public NoneApiDestroyable<RHIPipelineStatistics> {
		NonePipelineStatistics& fetch() override { return *this; }
	};

	struct NoneFence : public NoneApiDestroyable<RHIFence> {
		bool is_signaled() override { return true; }
		void reset() override {}
	};

	struct NoneSemaphore : public NoneApiDestroyable<RHISemaphore> {
	};

	struct NoneSampler : public NoneApiDestroyable<RHISampler> {
		RHIDescriptor descriptor() const override { return 0; }
	};

	struct NoneSRV : public RHIShaderResourceView {
		RHIDescriptor descriptor() const override { return 0; }
	};

	struct NoneUAV : public RHIUnorderedAccessView {
		RHIDescriptor descriptor() const override { return 0; }
	};

	struct NoneRTV : public RHIRenderTargetView {
	};

	struct NoneDSV : public RHIDepthStencilView {
	};

	struct NoneTexture : public NoneApiDestroyable<RHITexture> {
		RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc) override { return rhi_default<NoneRTV>(); }
		RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc) override { return rhi_default<NoneDSV>(); }
		RHIShaderResourceView* as_srv(RHITextureDescSRV* desc) override { return rhi_default<NoneSRV>(); }
		RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc) override { return rhi_default<NoneUAV>(); }
	};

	struct NoneShader : public NoneApiDestroyable<RHIShader> {
	};

	struct NonePipeline : public NoneApiDestroyable<RHIPipeline> {
	};

	struct NoneBuffer : public NoneApiDestroyable<RHIBuffer> {
		usize size() const override { return 0; }
		RHIDeviceAddress address() override { return 0; }
		u8* map(RHIMappingAccess access) override { return nullptr; }
		void unmap() override {}

		RHIShaderResourceView* as_srv(RHIBufferViewType type, RHIColorFormat format) override { return rhi_default<NoneSRV>(); }
		RHIUnorderedAccessView* as_uav(RHIBufferViewType type, RHIColorFormat format) override { return rhi_default<NoneUAV>(); }
	};

	struct NoneSwapchain : public NoneApiDestroyable<RHISwapchain> {
		void vsync(bool flag) override {}
		void resize(const Vector2u& new_size) override {}
		RHISemaphore* acquire_semaphore() override { return rhi_default<NoneSemaphore>(); }
		RHISemaphore* present_semaphore() override { return rhi_default<NoneSemaphore>(); }
		RHIRenderTargetView* as_rtv() override { return rhi_default<NoneRTV>(); }
		RHITexture* as_texture() override { return rhi_default<NoneTexture>(); }
	};

	struct NoneAccelerationStructure : public NoneApiDestroyable<RHIAccelerationStructure> {
	};

	NoneApi& NoneApi::update(float dt)
	{
		return *this;
	}

	NoneApi& NoneApi::submit(const RHISubmitInfo& info)
	{
		return *this;
	}

	NoneApi& NoneApi::idle()
	{
		return *this;
	}

	RHITimestamp* NoneApi::create_timestamp()
	{
		return new NoneTimestamp();
	}

	RHIPipelineStatistics* NoneApi::create_pipeline_statistics()
	{
		return new NonePipelineStatistics();
	}

	RHIFence* NoneApi::create_fence()
	{
		return new NoneFence();
	}

	RHISampler* NoneApi::create_sampler(const RHISamplerDesc& desc)
	{
		return new NoneSampler();
	}

	RHITexture* NoneApi::create_texture(const RHITextureDesc& desc)
	{
		return new NoneTexture();
	}

	RHIShader* NoneApi::create_shader(const u8* source, usize size)
	{
		return new NoneShader();
	}

	RHIPipeline* NoneApi::create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc)
	{
		return new NonePipeline();
	}

	RHIPipeline* NoneApi::create_mesh_pipeline(const RHIMeshPipelineDesc& desc)
	{
		return new NonePipeline();
	}

	RHIPipeline* NoneApi::create_compute_pipeline(const RHIComputePipelineDesc& desc)
	{
		return new NonePipeline();
	}

	RHIPipeline* NoneApi::create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc)
	{
		return new NonePipeline();
	}

	RHIBuffer* NoneApi::create_buffer(usize size, RHIBufferFlags flags)
	{
		return new NoneBuffer();
	}

	RHISwapchain* NoneApi::create_swapchain(Window* window, bool vsync)
	{
		return new NoneSwapchain();
	}

	RHIContext* NoneApi::create_context(RHIContextFlags flags)
	{
		return nullptr;
	}

	RHIAccelerationStructure* NoneApi::create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs)
	{
		return new NoneAccelerationStructure();
	}

	const u8* NoneApi::translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size)
	{
		size *= sizeof(RHIRayTracingGeometryInstance);
		return reinterpret_cast<const u8*>(instances);
	}

	NoneApi& NoneApi::present(RHISwapchain* swapchain)
	{
		return *this;
	}
}// namespace Trinex
