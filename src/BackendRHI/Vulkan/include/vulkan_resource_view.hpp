#pragma once
#include <Core/etl/set.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	class VulkanRenderTarget;
	class VulkanSampler;
	class VulkanTexture;
	class VulkanContext;
	class VulkanBuffer;
	class VulkanStateManager;


	class VulkanSRV : public RHIShaderResourceView
	{
	protected:
		RHIDescriptor m_descriptor;

	public:
		virtual VulkanSRV& bind(VulkanStateManager* manager, byte index) = 0;
		RHIDescriptor descriptor() const override;
	};

	class VulkanUAV : public RHIUnorderedAccessView
	{
	protected:
		RHIDescriptor m_descriptor;

	public:
		virtual VulkanUAV& bind(VulkanStateManager* manager, byte index) = 0;
		RHIDescriptor descriptor() const override;
	};

	class VulkanRTV : public RHIRenderTargetView
	{
	};

	class VulkanDSV : public RHIDepthStencilView
	{
	public:
	};


	class VulkanTextureSRV : public VulkanSRV
	{
	private:
		VulkanTexture* m_texture;
		vk::ImageView m_view;

	public:
		VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view);
		inline VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view, const VulkanTexture::ViewDesc&)
		    : VulkanTextureSRV(texture, view)
		{}
		~VulkanTextureSRV();
		VulkanSRV& bind(VulkanStateManager* manager, byte index) override;

		FORCE_INLINE VulkanTexture* texture() const { return m_texture; }
		FORCE_INLINE vk::ImageView view() const { return m_view; }
	};

	class VulkanTextureUAV : public VulkanUAV
	{
	private:
		VulkanTexture* m_texture;
		vk::ImageView m_view;

	public:
		VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view);
		inline VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view, const VulkanTexture::ViewDesc&)
		    : VulkanTextureUAV(texture, view)
		{}
		~VulkanTextureUAV();
		VulkanUAV& bind(VulkanStateManager* manager, byte index) override;

		FORCE_INLINE VulkanTexture* texture() const { return m_texture; }
		FORCE_INLINE vk::ImageView view() const { return m_view; }
	};

	class VulkanTextureRTV : public VulkanRTV
	{
	private:
		Set<VulkanRenderTarget*> m_render_targets;
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		uint16_t m_base_layer;
		uint16_t m_layer_count;
		uint16_t m_mip;

	public:
		VulkanTextureRTV(VulkanTexture* texture, vk::ImageView view, const VulkanTexture::ViewDesc& desc)
		    : m_texture(texture), m_view(view), m_base_layer(desc.first_array_slice), m_layer_count(desc.array_size),
		      m_mip(desc.first_mip)
		{}
		~VulkanTextureRTV();

		FORCE_INLINE vk::Extent3D extent() const { return m_texture->extent(); }
		FORCE_INLINE vk::Format format() const { return m_texture->format(); }
		FORCE_INLINE vk::Image image() const { return m_texture->image(); }
		FORCE_INLINE VulkanTexture* texture() const { return m_texture; }
		FORCE_INLINE vk::ImageView view() const { return m_view; }
		FORCE_INLINE uint16_t base_layer() const { return m_base_layer; }
		FORCE_INLINE uint16_t layer_count() const { return m_layer_count; }
		FORCE_INLINE uint16_t mip() const { return m_mip; }

		FORCE_INLINE VulkanTextureRTV& add_target(VulkanRenderTarget* target)
		{
			m_render_targets.insert(target);
			return *this;
		}

		FORCE_INLINE VulkanTextureRTV& remove_target(VulkanRenderTarget* target)
		{
			m_render_targets.erase(target);
			return *this;
		}
	};

	class VulkanTextureDSV : public VulkanDSV
	{
	private:
		Set<VulkanRenderTarget*> m_render_targets;
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		uint16_t m_base_layer;
		uint16_t m_layer_count;
		uint16_t m_mip;

	public:
		VulkanTextureDSV(VulkanTexture* texture, vk::ImageView view, const VulkanTexture::ViewDesc& desc)
		    : m_texture(texture), m_view(view), m_base_layer(desc.first_array_slice), m_layer_count(desc.array_size),
		      m_mip(desc.first_mip)
		{}
		~VulkanTextureDSV();

		FORCE_INLINE vk::Extent3D extent() const { return m_texture->extent(); }
		FORCE_INLINE vk::Format format() const { return m_texture->format(); }
		FORCE_INLINE vk::Image image() const { return m_texture->image(); }
		FORCE_INLINE VulkanTexture* texture() const { return m_texture; }
		FORCE_INLINE vk::ImageView view() const { return m_view; }
		FORCE_INLINE uint16_t base_layer() const { return m_base_layer; }
		FORCE_INLINE uint16_t layer_count() const { return m_layer_count; }
		FORCE_INLINE uint16_t mip() const { return m_mip; }

		FORCE_INLINE VulkanTextureDSV& add_target(VulkanRenderTarget* target)
		{
			m_render_targets.insert(target);
			return *this;
		}

		FORCE_INLINE VulkanTextureDSV& remove_target(VulkanRenderTarget* target)
		{
			m_render_targets.erase(target);
			return *this;
		}
	};

	class VulkanBufferSRV : public VulkanSRV
	{
	private:
		VulkanBuffer* m_buffer;

	public:
		inline VulkanBufferSRV(VulkanBuffer* buffer) : m_buffer(buffer) {}
		FORCE_INLINE VulkanBuffer* buffer() const { return m_buffer; };
	};

	class VulkanStorageBufferSRV : public VulkanBufferSRV
	{
	public:
		VulkanStorageBufferSRV(VulkanBuffer* buffer);
		~VulkanStorageBufferSRV();

		VulkanSRV& bind(VulkanStateManager* manager, byte index) override;
	};

	class VulkanUniformTexelBufferSRV : public VulkanBufferSRV
	{
	public:
		VulkanUniformTexelBufferSRV(VulkanBuffer* buffer);
		~VulkanUniformTexelBufferSRV();

		VulkanSRV& bind(VulkanStateManager* manager, byte index) override;
	};

	class VulkanBufferUAV : public VulkanUAV
	{
	private:
		VulkanBuffer* m_buffer;

	public:
		VulkanBufferUAV(VulkanBuffer* buffer);
		~VulkanBufferUAV();

		FORCE_INLINE VulkanBuffer* buffer() const { return m_buffer; };
		VulkanUAV& bind(VulkanStateManager* manager, byte index) override;
	};
}// namespace Engine
