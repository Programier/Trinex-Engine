#pragma once
#include <Core/math/vector.hpp>
#include <RHI/enums.hpp>
#include <RHI/object.hpp>
#include <RHI/types.hpp>

namespace Trinex
{
	class Archive;
	struct LinearColor;
	struct RHITextureDescRTV;
	struct RHITextureDescDSV;
	struct RHITextureDescSRV;
	struct RHITextureDescUAV;
	struct RHITextureType;
	struct RHIMappingAccess;

	class trinex_api RHITimestamp : public RHIObject
	{
	public:
		virtual float milliseconds() = 0;
	};

	class trinex_api RHIPipelineStatistics : public RHIObject
	{
	public:
		u64 vertices                                = 0;
		u64 primitives                              = 0;
		u64 vertex_shader_invocations               = 0;
		u64 geometry_shader_invocations             = 0;
		u64 geometry_shader_primitives              = 0;
		u64 clipping_invocations                    = 0;
		u64 clipping_primitives                     = 0;
		u64 fragment_shader_invocations             = 0;
		u64 tessellation_control_shader_invocations = 0;
		u64 tesselation_shader_invocations          = 0;

		virtual RHIPipelineStatistics& fetch() = 0;
	};

	class trinex_api RHIFence : public RHIObject
	{
	public:
		virtual bool is_signaled() = 0;
		virtual void reset()       = 0;
	};

	class trinex_api RHISemaphore : public RHIObject
	{
	public:
	};

	class trinex_api RHIShaderResourceView
	{
	public:
		virtual ~RHIShaderResourceView() {}
		virtual RHIDescriptor descriptor() const = 0;
	};

	class trinex_api RHIUnorderedAccessView
	{
	public:
		virtual ~RHIUnorderedAccessView() {}
		virtual RHIDescriptor descriptor() const = 0;
	};

	class trinex_api RHIRenderTargetView{public: virtual ~RHIRenderTargetView(){}};

	class trinex_api RHIDepthStencilView{public: virtual ~RHIDepthStencilView(){}};

	class trinex_api RHISampler : public RHIObject
	{
	public:
		virtual RHIDescriptor descriptor() const = 0;
	};

	class trinex_api RHITexture : public RHIObject
	{
	public:
		virtual Vector3u size() const                                             = 0;
		virtual RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc = nullptr)    = 0;
		virtual RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc = nullptr)    = 0;
		virtual RHIShaderResourceView* as_srv(RHITextureDescSRV* desc = nullptr)  = 0;
		virtual RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc = nullptr) = 0;
	};

	class trinex_api RHITextureView : public RHITexture
	{
	private:
		RHITexture* m_texture;
		u16 m_type;
		u16 m_base_slice;
		u16 m_slice_count;
		u16 m_base_mip;
		u16 m_mip_count;

	private:
		template<typename T>
		T initialize_description(const T* desc);

	public:
		RHITextureView(RHITexture* texture, RHITextureType type, u16 base_slice = 0, u16 slice_count = 65535, u16 base_mip = 0,
		               u16 mip_count = 65535);

		void destroy() override;

		RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc = nullptr) override;
		RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc = nullptr) override;
		RHIShaderResourceView* as_srv(RHITextureDescSRV* desc = nullptr) override;
		RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc = nullptr) override;
	};

	class trinex_api RHIShader : public RHIObject
	{
	};

	class trinex_api RHIPipeline : public RHIObject
	{
	};

	class trinex_api RHIBuffer : public RHIObject
	{
	public:
		bool serialize(Archive& ar);

		virtual usize size() const                                                                                        = 0;
		virtual RHIDeviceAddress address()                                                                                = 0;
		virtual u8* map(RHIMappingAccess access)                                                                          = 0;
		virtual void unmap()                                                                                              = 0;
		virtual RHIShaderResourceView* as_srv(RHIBufferViewType view, RHIColorFormat format = RHIColorFormat::Undefined)  = 0;
		virtual RHIUnorderedAccessView* as_uav(RHIBufferViewType view, RHIColorFormat format = RHIColorFormat::Undefined) = 0;
	};

	class trinex_api RHISwapchain : public RHIObject
	{
	public:
		virtual void present_interval(u32 interval) = 0;
		virtual void resize(const Vector2u& size)   = 0;

		virtual RHISemaphore* acquire_semaphore() = 0;
		virtual RHISemaphore* present_semaphore() = 0;
		virtual RHIRenderTargetView* as_rtv()     = 0;
		virtual RHITexture* as_texture()          = 0;
	};

	class trinex_api RHIAccelerationStructure : public RHIObject
	{
	public:
	};
} // namespace Trinex
