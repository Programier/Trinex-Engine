#pragma once
#include <Core/math/vector.hpp>
#include <RHI/types.hpp>

namespace Engine
{
	struct LinearColor;
	struct RHITextureDescRTV;
	struct RHITextureDescDSV;
	struct RHITextureDescSRV;
	struct RHITextureDescUAV;
	struct RHITextureType;

	class ENGINE_EXPORT RHIObject
	{
	private:
		static void static_release_internal(RHIObject* object);

	protected:
		size_t m_references;

	public:
		RHIObject(size_t init_ref_count = 1);
		virtual void add_reference();
		virtual void release();
		virtual void destroy() = 0;
		size_t references() const;
		virtual ~RHIObject();

		template<typename T>
		static inline void static_release(T* object)
		{
			if (object)
			{
				static_release_internal(object);
			}
		}

		template<typename T>
		T* as()
		{
			return static_cast<T*>(this);
		}

		template<typename T>
		const T* as() const
		{
			return static_cast<const T*>(this);
		}
	};

	class ENGINE_EXPORT RHITimestamp : public RHIObject
	{
	public:
		virtual bool is_ready()      = 0;
		virtual float milliseconds() = 0;
	};

	class ENGINE_EXPORT RHIPipelineStatistics : public RHIObject
	{
	public:
		virtual bool is_ready() = 0;

		virtual uint64_t vertices()                   = 0;
		virtual uint64_t primitives()                 = 0;
		virtual uint64_t geometry_shader_primitives() = 0;
		virtual uint64_t clipping_primitives()        = 0;

		virtual uint64_t vertex_shader_invocations()               = 0;
		virtual uint64_t tessellation_control_shader_invocations() = 0;
		virtual uint64_t tesselation_shader_invocations()          = 0;
		virtual uint64_t geometry_shader_invocations()             = 0;
		virtual uint64_t clipping_invocations()                    = 0;
		virtual uint64_t fragment_shader_invocations()             = 0;
	};

	class ENGINE_EXPORT RHIFence : public RHIObject
	{
	public:
		virtual bool is_signaled() = 0;
		virtual void reset()       = 0;
	};

	class ENGINE_EXPORT RHIShaderResourceView
	{
	public:
		virtual ~RHIShaderResourceView() {}
		virtual RHIDescriptor descriptor() const = 0;
	};

	class ENGINE_EXPORT RHIUnorderedAccessView
	{
	public:
		virtual ~RHIUnorderedAccessView() {}
		virtual RHIDescriptor descriptor() const = 0;
	};

	class ENGINE_EXPORT RHIRenderTargetView
	{
	public:
		virtual void clear(const LinearColor& color)   = 0;
		virtual void clear_uint(const Vector4u& value) = 0;
		virtual void clear_sint(const Vector4i& value) = 0;
		virtual ~RHIRenderTargetView() {}
	};

	class ENGINE_EXPORT RHIDepthStencilView
	{
	public:
		virtual void clear(float depth, byte stencil) = 0;
		virtual ~RHIDepthStencilView() {}
	};

	class ENGINE_EXPORT RHISampler : public RHIObject
	{
	public:
		virtual RHIDescriptor descriptor() const = 0;
	};

	class ENGINE_EXPORT RHITexture : public RHIObject
	{
	public:
		virtual RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc = nullptr)    = 0;
		virtual RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc = nullptr)    = 0;
		virtual RHIShaderResourceView* as_srv(RHITextureDescSRV* desc = nullptr)  = 0;
		virtual RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc = nullptr) = 0;
	};

	class ENGINE_EXPORT RHITextureView : public RHITexture
	{
	private:
		RHITexture* m_texture;
		uint16_t m_type;
		uint16_t m_base_slice;
		uint16_t m_slice_count;
		uint16_t m_base_mip;
		uint16_t m_mip_count;

	private:
		template<typename T>
		T initialize_description(const T* desc);

	public:
		RHITextureView(RHITexture* texture, RHITextureType type, uint16_t base_slice = 0, uint16_t slice_count = 65535,
		               uint16_t base_mip = 0, uint16_t mip_count = 65535);

		void destroy() override;
		
		RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc = nullptr) override;
		RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc = nullptr) override;
		RHIShaderResourceView* as_srv(RHITextureDescSRV* desc = nullptr) override;
		RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc = nullptr) override;
	};

	class ENGINE_EXPORT RHIShader : public RHIObject
	{
	};

	class ENGINE_EXPORT RHIPipeline : public RHIObject
	{
	public:
		virtual void bind() = 0;
	};

	class ENGINE_EXPORT RHIBuffer : public RHIObject
	{
	public:
		virtual byte* map()                                                            = 0;
		virtual void unmap()                                                           = 0;
		virtual RHIShaderResourceView* as_srv(uint32_t offset = 0, uint32_t size = 0)  = 0;
		virtual RHIUnorderedAccessView* as_uav(uint32_t offset = 0, uint32_t size = 0) = 0;
	};

	class RHISwapchain : public RHIObject
	{
	public:
		virtual void vsync(bool flag)             = 0;
		virtual void resize(const Vector2u& size) = 0;
		virtual RHIRenderTargetView* as_rtv()     = 0;
	};
}// namespace Engine
