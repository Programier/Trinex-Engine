#pragma once
#include <Core/etl/set.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanTexture : public VulkanDeferredDestroy<RHI_Texture>
	{
	private:
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		vk::Image m_image;
		vk::ImageLayout m_layout;
		RHITextureCreateFlags m_flags;

	public:
		virtual uint_t layer_count() const           = 0;
		virtual vk::ImageViewType view_type() const  = 0;
		virtual uint8_t mipmap_count() const         = 0;
		virtual vk::Format format() const            = 0;
		virtual RHIColorFormat engine_format() const = 0;
		virtual vk::ImageType image_type() const     = 0;
		virtual vk::Extent3D extent() const          = 0;

		vk::Image image() const;
		vk::ImageLayout layout() const;
		vk::ImageAspectFlags aspect() const;

		VulkanTexture& create(vk::ImageCreateFlagBits flags, vk::ImageUsageFlags usage, RHITextureCreateFlags create_flags);
		void change_layout(vk::ImageLayout new_layout);

		RHI_ShaderResourceView* create_srv();
		RHI_UnorderedAccessView* create_uav();
		RHI_RenderTargetView* create_rtv();
		RHI_DepthStencilView* create_dsv();

		byte* map(RHIMappingAccess access, const RHIMappingRange* range = nullptr) override;
		void unmap(const RHIMappingRange* range = nullptr) override;
		~VulkanTexture();
	};

	class VulkanTexture2D : public VulkanTexture
	{
	public:
		RHI_ShaderResourceView* m_srv  = nullptr;
		RHI_UnorderedAccessView* m_uav = nullptr;
		RHI_RenderTargetView* m_rtv    = nullptr;
		RHI_DepthStencilView* m_dsv    = nullptr;

		RHIColorFormat m_format;
		Vector2u m_size;
		uint8_t m_mips;

	public:
		VulkanTexture2D& create(RHIColorFormat format, Vector2u size, uint32_t mips, RHITextureCreateFlags flags);

		uint_t layer_count() const override;
		vk::ImageViewType view_type() const override;
		uint8_t mipmap_count() const override;
		vk::Format format() const override;
		RHIColorFormat engine_format() const override;
		vk::ImageType image_type() const override;
		vk::Extent3D extent() const override;

		~VulkanTexture2D();

		void update(byte mip, const RHIRect& rect, const byte* data, size_t data_size);
		inline RHI_ShaderResourceView* as_srv() override { return m_srv; }
		inline RHI_UnorderedAccessView* as_uav() override { return m_uav; }
		inline RHI_RenderTargetView* as_rtv() override { return m_rtv; }
		inline RHI_DepthStencilView* as_dsv() override { return m_dsv; }
	};
}// namespace Engine
