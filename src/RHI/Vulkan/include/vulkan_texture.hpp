#pragma once
#include <Graphics/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanTexture : RHI_DefaultDestroyable<RHI_Texture> {
	private:
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		vk::Image m_image;
		vk::ImageLayout m_layout;

	public:
		virtual uint_t layer_count() const                       = 0;
		virtual vk::ImageViewType view_type() const              = 0;
		virtual Vector2u size(MipMapLevel level = 0) const       = 0;
		virtual MipMapLevel mipmap_count() const                 = 0;
		virtual vk::Format format() const                        = 0;
		virtual ColorFormat engine_format() const                = 0;
		virtual vk::ImageType image_type() const                 = 0;
		virtual vk::Extent3D extent(MipMapLevel level = 0) const = 0;

		vk::Image image() const;
		vk::ImageLayout layout() const;
		vk::ImageAspectFlags aspect() const;

		VulkanTexture& create(vk::ImageCreateFlagBits flags, vk::ImageUsageFlags usage);
		void update_texture(const Vector2u& size, MipMapLevel level, uint_t layer, const byte* data, size_t data_size);
		VulkanTexture& layout(vk::ImageLayout layout);
		void change_layout(vk::ImageLayout new_layout);
		void change_layout(vk::ImageLayout new_layout, vk::CommandBuffer& cmd);

		~VulkanTexture();
	};

	struct VulkanTexture2D : VulkanTexture {
		const Texture2D* m_texture;

	public:
		VulkanTexture2D& create(const Texture2D* texture);

		uint_t layer_count() const override;
		vk::ImageViewType view_type() const override;
		Vector2u size(MipMapLevel level = 0) const override;
		MipMapLevel mipmap_count() const override;
		vk::Format format() const override;
		ColorFormat engine_format() const override;
		vk::ImageType image_type() const override;
		vk::Extent3D extent(MipMapLevel level = 0) const override;

		RHI_ShaderResourceView* create_srv() override;
		RHI_UnorderedAccessView* create_uav() override;
	};


	struct VulkanTextureSRV : public RHI_DefaultDestroyable<RHI_ShaderResourceView> {
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view);
		~VulkanTextureSRV();

		void bind(BindLocation location) override;
		void bind_combined(byte location, struct RHI_Sampler* sampler) override;
	};

	struct VulkanTextureUAV : public RHI_DefaultDestroyable<RHI_UnorderedAccessView> {
		VulkanTexture* m_texture;
		vk::ImageView m_view;

		VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view);
		~VulkanTextureUAV();

		void bind(BindLocation location) override;
	};
}// namespace Engine
